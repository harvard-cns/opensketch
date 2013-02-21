

  module header_parser
    #(parameter DATA_WIDTH = 64,
      parameter CTRL_WIDTH = DATA_WIDTH/8,
      parameter PKT_SIZE_WIDTH = 12,
      parameter ADDITIONAL_WORD_SIZE = 16,
      parameter ADDITIONAL_WORD_BITMASK = 16'h0fff,
      parameter ADDITIONAL_WORD_POS  = 224,
      parameter ADDITIONAL_WORD_CTRL = 8'h40,
      parameter ADDITIONAL_WORD_DEFAULT = 16'hffff,
      parameter FLOW_ENTRY_SIZE = 240
      )
   (// --- Interface to the previous stage
    input  [DATA_WIDTH-1:0]                in_data,
    input  [CTRL_WIDTH-1:0]                in_ctrl,
    input                                  in_wr,

    // --- Interface to matchers
    output reg [FLOW_ENTRY_SIZE-1:0]       flow_entry,
    output [`OPENSKETCH_ENTRY_SRC_PORT_WIDTH-1:0] flow_entry_src_port,
    output reg [PKT_SIZE_WIDTH-1:0]        pkt_size,
    output reg                             flow_entry_vld,

    // --- Misc
    input                                  reset,
    input                                  clk
   );

   `LOG2_FUNC

   //------------------ Internal Parameter ---------------------------
   localparam MODULE_HDRS                   = 0;
   localparam PKT_WORDS                     = 1;
   localparam WAIT_EOP                      = 2;
   localparam RESET_FLOW_ENTRY              = 3;

   localparam MAC_SRC_LO_ETHERTYPE_WORD     = 1,
              IP_PROT_WORD                  = 2,
              IP_SRC_DST_HI_WORD            = 3,
              IP_DST_LO_TRANSP_PORTS_WORD   = 4,
              ARP_DST_HI_WORD               = 5,
              ARP_DST_LO_WORD               = 6;

   //---------------------- Wires/Regs -------------------------------
   reg [1:0]                            state;
   reg [2:0]                            counter;

   reg                                  is_ip;
   reg                                  is_tcp_udp;
   reg                                  is_icmp;
   reg                                  is_vlan;
   reg                                  is_arp;

   reg [3:0]                            ip_hdr_len;

   wire [FLOW_ENTRY_SIZE-1:0]           flow_entry_default;

   //------------------------ Logic ----------------------------------
   assign flow_entry_src_port = flow_entry[`OPENSKETCH_ENTRY_SRC_PORT_POS + `OPENSKETCH_ENTRY_SRC_PORT_WIDTH - 1 : `OPENSKETCH_ENTRY_SRC_PORT_POS];
   generate
      if(FLOW_ENTRY_SIZE > ADDITIONAL_WORD_POS+ADDITIONAL_WORD_SIZE) begin:gen_flow_msb
         assign flow_entry_default[FLOW_ENTRY_SIZE-1:ADDITIONAL_WORD_POS+ADDITIONAL_WORD_SIZE]     = 0;
      end
   endgenerate
   assign flow_entry_default[ADDITIONAL_WORD_POS+ADDITIONAL_WORD_SIZE-1:ADDITIONAL_WORD_POS] = ADDITIONAL_WORD_DEFAULT;
   assign flow_entry_default[ADDITIONAL_WORD_POS-1:0]                                        = 0;

   /* This state machine parses the header */
   always @(posedge clk) begin
      if(reset) begin
         counter           <= 0;
         flow_entry_vld    <= 0;
         flow_entry        <= flow_entry_default;
         pkt_size          <= 0;
         state             <= MODULE_HDRS;
         is_ip             <= 0;
         is_tcp_udp        <= 0;
         is_icmp           <= 0;
         is_vlan           <= 0;
         ip_hdr_len        <= 0;
      end

      else begin
         flow_entry_vld    <= 0;
         case (state)
            RESET_FLOW_ENTRY: begin
               flow_entry    <= flow_entry_default;
               state         <= MODULE_HDRS;
               if(in_wr) begin
                  // check for the additional word
                  if(in_ctrl==ADDITIONAL_WORD_CTRL) begin
                     flow_entry[ADDITIONAL_WORD_POS + ADDITIONAL_WORD_SIZE - 1 : ADDITIONAL_WORD_POS]
                                                      <= in_data[ADDITIONAL_WORD_SIZE-1 : 0] & ADDITIONAL_WORD_BITMASK;
                     is_vlan <= (in_ctrl==`VLAN_CTRL_WORD);
                  end
                  // get the pkt size and the input port
                  else if(in_ctrl==`IO_QUEUE_STAGE_NUM) begin
                     flow_entry[`OPENSKETCH_ENTRY_SRC_PORT_POS + `OPENSKETCH_ENTRY_SRC_PORT_WIDTH - 1 : `OPENSKETCH_ENTRY_SRC_PORT_POS]
                                                          <= in_data[`IOQ_SRC_PORT_POS + `OPENSKETCH_ENTRY_SRC_PORT_WIDTH - 1 : `IOQ_SRC_PORT_POS];
                     if(is_vlan) begin
                        pkt_size <= (in_data[`IOQ_BYTE_LEN_POS + PKT_SIZE_WIDTH - 1:`IOQ_BYTE_LEN_POS] + 4);
                     end
                     else begin
                        pkt_size <= in_data[`IOQ_BYTE_LEN_POS + PKT_SIZE_WIDTH - 1:`IOQ_BYTE_LEN_POS];
                     end
                     is_vlan <= 0;
                  end
                  // pkt should not be started
                  // synthesis translate_off
                  else if(in_ctrl==0) begin
                     $display("%t %m ERROR: found ctrl=0 as first word of pkt.", $time);
                     $stop;
                  end
                  // synthesis translate_on
               end // if (in_wr)
            end // case: RESET_FLOW_ENTRY

            MODULE_HDRS: begin
               if(in_wr) begin
                  // check for the additional word
                  if(in_ctrl==ADDITIONAL_WORD_CTRL) begin
                     flow_entry[ADDITIONAL_WORD_POS + ADDITIONAL_WORD_SIZE - 1 : ADDITIONAL_WORD_POS]
                                                      <= in_data[ADDITIONAL_WORD_SIZE-1 : 0] & ADDITIONAL_WORD_BITMASK;
                     is_vlan <= (in_ctrl==`VLAN_CTRL_WORD);
                  end
                  // get the pkt size and the input port
                  else if(in_ctrl==`IO_QUEUE_STAGE_NUM) begin
                     flow_entry[`OPENSKETCH_ENTRY_SRC_PORT_POS + `OPENSKETCH_ENTRY_SRC_PORT_WIDTH - 1:`OPENSKETCH_ENTRY_SRC_PORT_POS]
                                                          <= in_data[`IOQ_SRC_PORT_POS + `OPENSKETCH_ENTRY_SRC_PORT_WIDTH - 1:`IOQ_SRC_PORT_POS];
                     if(is_vlan) begin
                        pkt_size <= (in_data[`IOQ_BYTE_LEN_POS + PKT_SIZE_WIDTH - 1:`IOQ_BYTE_LEN_POS] + 4);
                     end
                     else begin
                        pkt_size <= in_data[`IOQ_BYTE_LEN_POS + PKT_SIZE_WIDTH - 1:`IOQ_BYTE_LEN_POS];
                     end
                     is_vlan <= 0;
                  end
                  // pkt started - get MAC dst and src hi
                  else if(in_ctrl==0) begin
                     flow_entry[`OPENSKETCH_ENTRY_ETH_DST_POS + 47 : `OPENSKETCH_ENTRY_ETH_DST_POS]
                                        <= in_data[63:16];

                     flow_entry[`OPENSKETCH_ENTRY_ETH_SRC_POS + 47 : `OPENSKETCH_ENTRY_ETH_SRC_POS + 32]
                                        <= in_data[15:0];
                     state    <= PKT_WORDS;
                  end
                  counter <= 1;
               end // if (in_wr)
            end // case: MODULE_HDRS

            PKT_WORDS: begin
               if(in_wr) begin

                  counter <= counter + 1;

                  case(counter)
                     MAC_SRC_LO_ETHERTYPE_WORD: begin
                        flow_entry[`OPENSKETCH_ENTRY_ETH_SRC_POS + 31 : `OPENSKETCH_ENTRY_ETH_SRC_POS]
                                                        <= in_data[63:32];

                        flow_entry[`OPENSKETCH_ENTRY_ETH_TYPE_POS + 15 : `OPENSKETCH_ENTRY_ETH_TYPE_POS]
                                                        <= in_data[31:16];

                        if (in_data[31:16] == `ETH_TYPE_IP) begin
                           flow_entry[`OPENSKETCH_ENTRY_IP_TOS_POS + 7 : `OPENSKETCH_ENTRY_IP_TOS_POS + 2]
                                                           <= in_data[7:2];
                        end

                        is_ip      <= in_data[31:16] == `ETH_TYPE_IP;
                        is_arp     <= in_data[31:16] == `ETH_TYPE_ARP;
                        ip_hdr_len <= in_data[11:8];
                     end

                     IP_PROT_WORD: begin
                        if(is_ip) begin
                           flow_entry[`OPENSKETCH_ENTRY_IP_PROTO_POS + 7 : `OPENSKETCH_ENTRY_IP_PROTO_POS] <= in_data[7:0];
                           /* check validity of hdr length field */
                           if(ip_hdr_len < 5) begin
                              flow_entry        <= flow_entry_default;
                              flow_entry_vld    <= 1'b1;
                              state             <= WAIT_EOP;
                           end
                        end
                        else if(is_arp) begin
                           flow_entry[`OPENSKETCH_ENTRY_IP_PROTO_POS + 7 : `OPENSKETCH_ENTRY_IP_PROTO_POS] <= in_data[23:16];
                        end
                        is_tcp_udp <= is_ip && ((in_data[7:0] == `IP_PROTO_TCP)
                                                || (in_data[7:0] == `IP_PROTO_UDP));
                        is_icmp    <= is_ip && (in_data[7:0] == `IP_PROTO_ICMP);
                     end

                     IP_SRC_DST_HI_WORD: begin
                        if(is_ip) begin
                           flow_entry[`OPENSKETCH_ENTRY_IP_SRC_POS + 31:`OPENSKETCH_ENTRY_IP_SRC_POS]
				    <= in_data[47:16];
                           flow_entry[`OPENSKETCH_ENTRY_IP_DST_POS + 31: `OPENSKETCH_ENTRY_IP_DST_POS + 16]
			            <= in_data[15:0];
                        end
                        else if(is_arp) begin
                           counter <= counter + 2; // jump to ARP_DST_HI_WORD state
                           flow_entry[`OPENSKETCH_ENTRY_IP_SRC_POS + 31:`OPENSKETCH_ENTRY_IP_SRC_POS]
				    <= in_data[31:0];
                        end
                     end

                     IP_DST_LO_TRANSP_PORTS_WORD: begin
                        counter       <= counter; // stay in this state to get tcp/udp ports
                        ip_hdr_len    <= ip_hdr_len - 2'h2; // keep track of words

                        if(is_ip) begin
                           flow_entry[`OPENSKETCH_ENTRY_IP_DST_POS + 15:`OPENSKETCH_ENTRY_IP_DST_POS]
 				    <= in_data[63:48];
                           is_ip    <= 0;
                        end

                        if(is_tcp_udp) begin
                           if(ip_hdr_len == 5) begin
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_SRC_POS + 15:`OPENSKETCH_ENTRY_TRANSP_SRC_POS]    <= in_data[47:32];
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_DST_POS + 15:`OPENSKETCH_ENTRY_TRANSP_DST_POS]    <= in_data[31:16];
                           end // if (ip_hdr_len == 5)
                           else if(ip_hdr_len == 6) begin
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_SRC_POS + 15:`OPENSKETCH_ENTRY_TRANSP_SRC_POS]    <= in_data[15:0];
                           end
                           else if(ip_hdr_len == 4) begin
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_DST_POS + 15:`OPENSKETCH_ENTRY_TRANSP_DST_POS]    <= in_data[63:48];
                           end // if (ip_hdr_len == 4)
                        end
                        else if (is_icmp) begin
                           if(ip_hdr_len == 5) begin
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_SRC_POS + 15:`OPENSKETCH_ENTRY_TRANSP_SRC_POS + 8] <= 8'h0;
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_SRC_POS +  7:`OPENSKETCH_ENTRY_TRANSP_SRC_POS]     <= in_data[47:40];
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_DST_POS + 15:`OPENSKETCH_ENTRY_TRANSP_DST_POS + 8] <= 8'h0;
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_DST_POS +  7:`OPENSKETCH_ENTRY_TRANSP_DST_POS]     <= in_data[39:32];
                           end // if (ip_hdr_len == 5)
                           else if(ip_hdr_len == 6) begin
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_SRC_POS + 15:`OPENSKETCH_ENTRY_TRANSP_SRC_POS + 8] <= 8'h0;
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_SRC_POS +  7:`OPENSKETCH_ENTRY_TRANSP_SRC_POS]     <= in_data[15:8];
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_DST_POS + 15:`OPENSKETCH_ENTRY_TRANSP_DST_POS + 8] <= 8'h0;
                              flow_entry[`OPENSKETCH_ENTRY_TRANSP_DST_POS +  7:`OPENSKETCH_ENTRY_TRANSP_DST_POS]     <= in_data[7:0];
                           end // if (ip_hdr_len == 6).  if(ip_hder_len ==4), nothing to do.
                        end // if ((is_tcp_udp || is_icmp) && ip_hdr_len >= 4)
                        if(!(is_tcp_udp || is_icmp)
                           || ((is_tcp_udp || is_icmp)
                               && (ip_hdr_len == 5
                                   || ip_hdr_len == 4))) begin
                           flow_entry_vld    <= 1'b1;
                           /* check for eop */
                           if(in_ctrl != 0) begin
                              state    <= RESET_FLOW_ENTRY;
                           end
                           else begin
                              state    <= WAIT_EOP;
                           end
                        end // else: !if(is_tcp_udp || is_icmp)
                     end // case: IP_DST_LO_TRANSP_PORTS_WORD

                     ARP_DST_HI_WORD: begin
                        flow_entry[`OPENSKETCH_ENTRY_IP_DST_POS + 31:`OPENSKETCH_ENTRY_IP_DST_POS + 16]
                           <= in_data[15:0];
                     end

                     default: begin
                        flow_entry[`OPENSKETCH_ENTRY_IP_DST_POS + 15:`OPENSKETCH_ENTRY_IP_DST_POS]
                           <= in_data[63:48];
                        is_arp            <= 0;
                        flow_entry_vld    <= 1'b1;
                        /* check for eop */
                        if(in_ctrl != 0) begin
                           state    <= RESET_FLOW_ENTRY;
                        end
                        else begin
                           state    <= WAIT_EOP;
                        end
                     end // case: ARP_DST_LO_WORD

                  endcase // case(counter)
               end // if (in_wr)
            end // case: PKT_WORDS

            WAIT_EOP: begin
               flow_entry_vld    <= 1'b0;
               if(in_ctrl!=0 & in_wr) begin
                  state             <= MODULE_HDRS;
                  flow_entry        <= flow_entry_default;
                  pkt_size          <= 0;
               end
            end

         endcase // case(state)
      end // else: !if(reset)
   end // always @ (posedge clk)

endmodule // header_parser

