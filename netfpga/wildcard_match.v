
  module wildcard_match
    #(parameter PKT_SIZE_WIDTH = 12,                    // number of bits for pkt size
      parameter UDP_REG_SRC_WIDTH = 2                   // identifies which module started this request
		)
   (// --- Interface for lookups
    input [`OPENSKETCH_HASH_INDEX_WIDTH-1:0]    flow_entry,
    input                                  flow_entry_vld,
    input [PKT_SIZE_WIDTH-1:0]             pkt_size,
    output                                 wildcard_match_rdy,

    // --- Interface to arbiter
    output                                 wildcard_hit,
    output                                 wildcard_miss,
    output [`OPENSKETCH_ACTION_WIDTH-1:0]    wildcard_data,
    output                                 wildcard_data_vld,
    input                                  wildcard_wins,
    input                                  wildcard_loses,

    // --- Interface to registers
    input                                  reg_req_in,
    input                                  reg_ack_in,
    input                                  reg_rd_wr_L_in,
    input  [`UDP_REG_ADDR_WIDTH-1:0]       reg_addr_in,
    input  [`CPCI_NF2_DATA_WIDTH-1:0]      reg_data_in,
    input  [UDP_REG_SRC_WIDTH-1:0]         reg_src_in,

    output                                 reg_req_out,
    output                                 reg_ack_out,
    output                                 reg_rd_wr_L_out,
    output     [`UDP_REG_ADDR_WIDTH-1:0]   reg_addr_out,
    output     [`CPCI_NF2_DATA_WIDTH-1:0]  reg_data_out,
    output     [UDP_REG_SRC_WIDTH-1:0]     reg_src_out,

    // --- Interface to Watchdog Timer
    //input                                  table_flush,

    // --- Misc
    //input [31:0]                           openflow_timer,
	 input											 fill,
	 //output [31:0]									 rule[`OPENSKETCH_WILDCARD_TABLE_SIZE-1:0],
    input                                  reset,
    input                                  clk
   );

   `LOG2_FUNC
   `CEILDIV_FUNC

   //-------------------- Internal Parameters ------------------------
   localparam WILDCARD_NUM_DATA_WORDS_USED = `OPENSKETCH_WILDCARD_NUM_DATA_WORDS_USED;
   localparam WILDCARD_NUM_CMP_WORDS_USED  = `OPENSKETCH_WILDCARD_NUM_CMP_WORDS_USED;
   localparam WILDCARD_NUM_REGS_USED = (2 // for the read and write address registers
                                        + `OPENSKETCH_WILDCARD_NUM_DATA_WORDS_USED // for data associated with an entry
                                        + `OPENSKETCH_WILDCARD_NUM_CMP_WORDS_USED  // for the data to match on
                                        + `OPENSKETCH_WILDCARD_NUM_CMP_WORDS_USED  // for the don't cares
                                        );

   localparam LUT_DEPTH_BITS = log2(`OPENSKETCH_WILDCARD_TABLE_SIZE);

   localparam SIMULATION = 0
	      // synthesis translate_off
	      || 1
	      // synthesis translate_on
	      ;


   //---------------------- Wires and regs----------------------------
   wire                                                      cam_busy;
   wire                                                      cam_match;
   wire [`OPENSKETCH_WILDCARD_TABLE_SIZE-1:0]                cam_match_addr;
   wire [`OPENSKETCH_HASH_INDEX_WIDTH-1:0]                        cam_cmp_din, cam_cmp_data_mask;
   wire [`OPENSKETCH_HASH_INDEX_WIDTH-1:0]                        cam_din, cam_data_mask;
   wire [`OPENSKETCH_WILDCARD_NUM_SUBTABLE-1:0]              cam_we;
   wire [LUT_DEPTH_BITS-1:0]                                 cam_wr_addr;

   wire [WILDCARD_NUM_CMP_WORDS_USED * `OPENSKETCH_WILDCARD_NUM_SUBTABLE-1:0] cam_busy_ind;
   wire [WILDCARD_NUM_CMP_WORDS_USED * `OPENSKETCH_WILDCARD_NUM_SUBTABLE-1:0] cam_match_ind;
   wire [WILDCARD_NUM_CMP_WORDS_USED-1:0]                    cam_match_addr_ind[`OPENSKETCH_WILDCARD_TABLE_SIZE-1:0];
   wire [31:0]                                               cam_cmp_din_ind[WILDCARD_NUM_CMP_WORDS_USED -1:0];
   wire [31:0]                                               cam_cmp_data_mask_ind[WILDCARD_NUM_CMP_WORDS_USED -1:0];
   wire [31:0]                                               cam_din_ind[WILDCARD_NUM_CMP_WORDS_USED -1:0];
   wire [31:0]                                               cam_data_mask_ind[WILDCARD_NUM_CMP_WORDS_USED -1:0];

   wire [`UDP_REG_ADDR_WIDTH-1:0]                            cam_reg_addr_out;
   wire [`CPCI_NF2_DATA_WIDTH-1:0]                           cam_reg_data_out;
   wire [UDP_REG_SRC_WIDTH-1:0]                              cam_reg_src_out;

   wire [LUT_DEPTH_BITS-1:0]                                 wildcard_address;
   wire [LUT_DEPTH_BITS-1:0]                                 dout_wildcard_address;

   //reg [`OPENSKETCH_WILDCARD_TABLE_SIZE-1:0]                   wildcard_hit_address_decoded;
   //wire [`OPENSKETCH_WILDCARD_TABLE_SIZE*PKT_SIZE_WIDTH - 1:0] wildcard_hit_address_decoded_expanded;
   //wire [`OPENSKETCH_WILDCARD_TABLE_SIZE*PKT_SIZE_WIDTH - 1:0] wildcard_entry_hit_byte_size;
   //wire [`OPENSKETCH_WILDCARD_TABLE_SIZE*32 - 1:0]             wildcard_entry_last_seen_timestamps;

   wire [PKT_SIZE_WIDTH-1:0]                                 dout_pkt_size;

   //reg [PKT_SIZE_WIDTH-1:0]                                  wildcard_entry_hit_byte_size_word [`OPENSKETCH_WILDCARD_TABLE_SIZE-1:0];
   //reg [31:0]                                                wildcard_entry_last_seen_timestamps_words[`OPENSKETCH_WILDCARD_TABLE_SIZE-1:0];

	
   integer                                                   i;

   //------------------------- Modules -------------------------------
   assign wildcard_match_rdy = 1;

   unencoded_cam_lut_sm
     #(.CMP_WIDTH (`OPENSKETCH_HASH_INDEX_WIDTH),
       .DATA_WIDTH (`OPENSKETCH_ACTION_WIDTH),
       .LUT_DEPTH  (`OPENSKETCH_WILDCARD_TABLE_SIZE),
       .TAG (`OPENSKETCH_WILDCARD_LOOKUP_BLOCK_ADDR),
       .REG_ADDR_WIDTH (`OPENSKETCH_WILDCARD_LOOKUP_REG_ADDR_WIDTH),
		 .NUM_SUB_TABLE(`OPENSKETCH_WILDCARD_NUM_SUBTABLE))
       wildcard_cam_lut_sm
         (// --- Interface for lookups
          .lookup_req          (flow_entry_vld),
          .lookup_cmp_data     (flow_entry),
          .lookup_cmp_dmask    ({`OPENSKETCH_HASH_INDEX_WIDTH{1'b0}}),
          .lookup_ack          (wildcard_data_vld),
          .lookup_hit          (wildcard_hit),
          .lookup_data         (wildcard_data),
          .lookup_address      (wildcard_address),

          // --- Interface to registers
          .reg_req_in          (reg_req_in),
          .reg_ack_in          (reg_ack_in),
          .reg_rd_wr_L_in      (reg_rd_wr_L_in),
          .reg_addr_in         (reg_addr_in),
          .reg_data_in         (reg_data_in),
          .reg_src_in          (reg_src_in),

          .reg_req_out         (cam_reg_req_out),
          .reg_ack_out         (cam_reg_ack_out),
          .reg_rd_wr_L_out     (cam_reg_rd_wr_L_out),
          .reg_addr_out        (cam_reg_addr_out),
          .reg_data_out        (cam_reg_data_out),
          .reg_src_out         (cam_reg_src_out),

          // --- CAM interface
          .cam_busy            (cam_busy),
          .cam_match           (cam_match),
          .cam_match_addr      (cam_match_addr),
          .cam_cmp_din         (cam_cmp_din),
          .cam_din             (cam_din),
          .cam_we              (cam_we),
          .cam_wr_addr         (cam_wr_addr),
          .cam_cmp_data_mask   (cam_cmp_data_mask),
          .cam_data_mask       (cam_data_mask),

          // --- Watchdog Timer Interface
          //.table_flush         (table_flush),

          // --- Misc
			 .fill					 (fill),
			 //.rule					 (rule),
          .reset               (reset),
          .clk                 (clk));

   /* Split up the CAM into multiple smaller CAMs to improve timing */
   generate
      genvar ii,j,jj;
		for (jj=0; jj<`OPENSKETCH_WILDCARD_NUM_SUBTABLE; jj=jj+1) begin: gen_subtable
			for (ii=0; ii<WILDCARD_NUM_CMP_WORDS_USED; ii=ii+1) begin:gen_cams
				wire [31:0] cam_match_addr_temp;
				srl_cam_unencoded_32x32 openflow_cam
				  (
					// Outputs
					.busy                             (cam_busy_ind[jj * `OPENSKETCH_WILDCARD_NUM_CMP_WORDS_USED + ii]),
					.match                            (cam_match_ind[jj *`OPENSKETCH_WILDCARD_NUM_CMP_WORDS_USED + ii]),
					.match_addr                       (cam_match_addr_temp),
					// Inputs
					.clk                              (clk),
					.cmp_din                          (cam_cmp_din_ind[ii]),  //sharing data bus 
					.din                              (cam_din_ind[ii]),
					.cmp_data_mask                    (cam_cmp_data_mask_ind[ii]),
					.data_mask                        (cam_data_mask_ind[ii]),
					.we                               (cam_we[jj]),		//use the we signal to isolate compare table writing
					.wr_addr                          (cam_wr_addr[4:0])		//sharing addr bus
					);
				if(ii < WILDCARD_NUM_CMP_WORDS_USED - 1) begin
					assign cam_cmp_din_ind[ii]         = cam_cmp_din[32*ii + 31: 32*ii];
					assign cam_din_ind[ii]             = cam_din[32*ii + 31: 32*ii];
					assign cam_cmp_data_mask_ind[ii]   = cam_cmp_data_mask[32*ii + 31: 32*ii];
					assign cam_data_mask_ind[ii]       = cam_data_mask[32*ii + 31: 32*ii];
					//assign cam_cmp_din_ind[ii]         = cam_cmp_din[32*ii + 31: 32*ii];
				end
				else begin
					assign cam_cmp_din_ind[ii]         = cam_cmp_din[`OPENSKETCH_HASH_INDEX_WIDTH-1: 32*ii];
					assign cam_din_ind[ii]             = cam_din[`OPENSKETCH_HASH_INDEX_WIDTH-1: 32*ii];
					assign cam_cmp_data_mask_ind[ii]   = cam_cmp_data_mask[`OPENSKETCH_HASH_INDEX_WIDTH-1: 32*ii];
					assign cam_data_mask_ind[ii]       = cam_data_mask[`OPENSKETCH_HASH_INDEX_WIDTH-1: 32*ii];
					//assign cam_cmp_din_ind[ii]         = cam_cmp_din[`OPENSKETCH_HASH_INDEX_WIDTH-1: 32*ii];
				end // else: !if(ii < WILDCARD_NUM_CMP_WORDS_USED - 1)

				for (j=0; j<32; j=j+1) begin:gen_match_addr_mem
					assign cam_match_addr_ind[jj*32 + j][ii] = cam_match_addr_temp[j];
				end
			end // block: gen_cams
		end //block: gen_subtable

      for (ii=0; ii<`OPENSKETCH_WILDCARD_TABLE_SIZE; ii=ii+1) begin:gen_match_addr
         assign cam_match_addr[ii] = &cam_match_addr_ind[ii];
      end
   endgenerate

   assign cam_busy  = |cam_busy_ind;
   assign cam_match = |cam_match_addr;

//   generic_regs
//     #(.UDP_REG_SRC_WIDTH (UDP_REG_SRC_WIDTH),
//       .TAG (`OPENSKETCH_WILDCARD_LOOKUP_BLOCK_ADDR),
//       .REG_ADDR_WIDTH (`OPENSKETCH_WILDCARD_LOOKUP_REG_ADDR_WIDTH),
//       .NUM_COUNTERS (`OPENSKETCH_WILDCARD_TABLE_SIZE  // for number of bytes
//                      +`OPENSKETCH_WILDCARD_TABLE_SIZE // for number of packets
//                      ),
//       /*****************
//	* JN: FIXME For now we will reset on read during simulation only
//	*****************/
//       .RESET_ON_READ (SIMULATION),
//       .NUM_SOFTWARE_REGS (2),
//       .NUM_HARDWARE_REGS (`OPENSKETCH_WILDCARD_TABLE_SIZE), // for last seen timestamps
//       .COUNTER_INPUT_WIDTH (PKT_SIZE_WIDTH), // max pkt size
//       .REG_START_ADDR (WILDCARD_NUM_REGS_USED) // used for the access to the cam/lut
//       )
//   generic_regs
//     (
//      .reg_req_in        (cam_reg_req_out),
//      .reg_ack_in        (cam_reg_ack_out),
//      .reg_rd_wr_L_in    (cam_reg_rd_wr_L_out),
//      .reg_addr_in       (cam_reg_addr_out),
//      .reg_data_in       (cam_reg_data_out),
//      .reg_src_in        (cam_reg_src_out),
//
//      .reg_req_out       (reg_req_out),
//      .reg_ack_out       (reg_ack_out),
//      .reg_rd_wr_L_out   (reg_rd_wr_L_out),
//      .reg_addr_out      (reg_addr_out),
//      .reg_data_out      (reg_data_out),
//      .reg_src_out       (reg_src_out),
//
//      // --- counters interface
//      .counter_updates   (/*{wildcard_hit_address_decoded_expanded,
//                           wildcard_entry_hit_byte_size}*/
//                          ),
//      .counter_decrement (//{(2*`OPENSKETCH_WILDCARD_TABLE_SIZE){1'b0}}
//									),
//
//      // --- SW regs interface
//      .software_regs     (),
//
//      // --- HW regs interface
//      .hardware_regs     (//{wildcard_entry_last_seen_timestamps}
//									),
//
//      .clk               (clk),
//      .reset             (reset));

   /* we might receive four input packets simultaneously from ethernet. In addition,
    * we might receive a pkt from DMA. So we need at least 5 spots. */
/*	 
   fallthrough_small_fifo
     #(.WIDTH(PKT_SIZE_WIDTH),
       .MAX_DEPTH_BITS(3))
      pkt_size_fifo
        (.din           (pkt_size),
         .wr_en         (flow_entry_vld),
         .rd_en         (fifo_rd_en),
         .dout          (dout_pkt_size),
         .full          (),
         .nearly_full   (),
         .empty         (pkt_size_fifo_empty),
         .reset         (reset),
         .clk           (clk)
         );

	
   fallthrough_small_fifo
     #(.WIDTH(LUT_DEPTH_BITS),
       .MAX_DEPTH_BITS(3))
      address_fifo
        (.din           (wildcard_address),
         .wr_en         (wildcard_data_vld),
         .rd_en         (fifo_rd_en),
         .dout          (dout_wildcard_address),
         .full          (),
         .nearly_full   (),
         .empty         (address_fifo_empty),
         .reset         (reset),
         .clk           (clk)
         );
*/
   //-------------------------- Logic --------------------------------
   assign wildcard_miss = wildcard_data_vld & !wildcard_hit;
   assign fifo_rd_en = wildcard_wins || wildcard_loses;

   /* update the generic register interface if wildcard matching
    * wins the arbitration */
/*   always @(*) begin
      wildcard_hit_address_decoded = 0;
      for(i=0; i<`OPENSKETCH_WILDCARD_TABLE_SIZE; i=i+1) begin
         wildcard_entry_hit_byte_size_word[i] = 0;
      end
      if(wildcard_wins) begin
         wildcard_hit_address_decoded[dout_wildcard_address] = 1;
         wildcard_entry_hit_byte_size_word[dout_wildcard_address]
           = dout_pkt_size;
      end
   end // always @ (*)

   generate
      genvar gi;
      for(gi=0; gi<`OPENSKETCH_WILDCARD_TABLE_SIZE; gi=gi+1) begin:concat
         assign wildcard_entry_hit_byte_size[gi*PKT_SIZE_WIDTH +: PKT_SIZE_WIDTH]
                = wildcard_entry_hit_byte_size_word[gi];
         assign wildcard_entry_last_seen_timestamps[gi*32 +: 32]
                = wildcard_entry_last_seen_timestamps_words[gi];
         assign wildcard_hit_address_decoded_expanded[gi*PKT_SIZE_WIDTH +: PKT_SIZE_WIDTH]
                ={{(PKT_SIZE_WIDTH-1){1'b0}}, wildcard_hit_address_decoded[gi]};
      end
   endgenerate

   // update the timestamp of the entry
   always @(posedge clk) begin
      if(cam_we) begin
         wildcard_entry_last_seen_timestamps_words[cam_wr_addr] <= openflow_timer;
      end
      else if(wildcard_wins) begin
         wildcard_entry_last_seen_timestamps_words[dout_wildcard_address] <= openflow_timer;
      end
   end // always @ (posedge clk)*/

endmodule // wildcard_match


