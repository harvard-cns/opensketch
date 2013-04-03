// This module hashnmeasure should be modified 
// by adding further instantiation of hardware blocks to create your architecture 

module hashnmeasure #(
      parameter DATA_WIDTH = 64,
      parameter CTRL_WIDTH = DATA_WIDTH/8,
      parameter UDP_REG_SRC_WIDTH = 2
   )
 (  

      input  [DATA_WIDTH-1:0]              in_data,
      input  [CTRL_WIDTH-1:0]              in_ctrl,
      input                                in_wr,
      output                               in_rdy,

      output [DATA_WIDTH-1:0]              out_data,
      output [CTRL_WIDTH-1:0]              out_ctrl,
      output                               out_wr,
      input                                out_rdy,
     

      // --- Register interface
      input                               reg_req_in,
      input                               reg_ack_in,
      input                               reg_rd_wr_L_in,
      input  [`UDP_REG_ADDR_WIDTH-1:0]    reg_addr_in,
      input  [`CPCI_NF2_DATA_WIDTH-1:0]   reg_data_in,
      input  [UDP_REG_SRC_WIDTH-1:0]      reg_src_in,

      output                              reg_req_out,
      output                              reg_ack_out,
      output                              reg_rd_wr_L_out,
      output  [`UDP_REG_ADDR_WIDTH-1:0]   reg_addr_out,
      output  [`CPCI_NF2_DATA_WIDTH-1:0]  reg_data_out,
      output  [UDP_REG_SRC_WIDTH-1:0]     reg_src_out,

		//test-only
		output 										wildcard_hit,
		output 										wildcard_miss,
		output [`OPENSKETCH_ACTION_WIDTH-1:0] wildcard_data,
		output										wildcard_data_vld,
		

		// ---- SRAM 1
		 // Note: 1 extra address bit on sram
		 output [19:0] 							sram1_addr,
		 inout  [`SRAM_DATA_WIDTH - 1:0] 	sram1_data,
		 output        							sram1_we,
		 output [3:0]  							sram1_bw,
		 output     								sram1_zz,

		 // ---- SRAM 2
		 // Note: 1 extra address bit on sram
		 output [19:0] 							sram2_addr,
		 inout  [`SRAM_DATA_WIDTH - 1:0] 	sram2_data,
		 output        							sram2_we,
		 output [3:0]  							sram2_bw,
		 output     								sram2_zz,
	 
	 
	 output [DATA_WIDTH+CTRL_WIDTH-1:0] sram_out,
		
		input                                reset,
		input                                clk
 );


  
   wire [DATA_WIDTH-1:0]         in_fifo_data;
   wire [CTRL_WIDTH-1:0]         in_fifo_ctrl;

   wire                          in_fifo_nearly_full;
   wire                          in_fifo_empty;
   reg                           out_wr_int;
   reg                           in_fifo_rd_en;

// Wires for HW/SW registers
	wire [31:0]  		filed_id, numHashValues, range;
	wire [31:0]  		rev_filed_id, rev_numHashValues, rev_range;
	wire [31:0]			numRows, countersPerRow, counterSize, updateType;
	wire [31:0]			addrfunc_list, hash_idx, i, func_list, constant_value;
	wire [31:0]			sram_size;
	
	
	
	wire [31:0] 		countersPerRow2, counterSize2, i2, hash_idx2;
	
	//header parse
	wire [`OPENSKETCH_ENTRY_WIDTH-1:0] flow_entry;
	wire [7:0] flow_entry_src_port;
	wire [11:0] pkt_size;
	wire flow_entry_vld;
	
	//entry hash
	wire [31:0] flow_index_0;
	wire [31:0] flow_index_1;
	wire [31:0] flow_index_2;
	wire [31:0] flow_index_3;
	wire [31:0] flow_index_4;
	wire [31:0] flow_index_5;
	wire [31:0] flow_index_6;
	wire [31:0] flow_index_7;
	wire [31:0] flow_index_8;
	wire [31:0] flow_index_9;
	
	reg [31:0] hash;

	wire [11:0] dout_pkt_size;
	
	
	//entry fifo
	wire [`OPENSKETCH_ENTRY_WIDTH-1:0] dout_flow_entry;
	reg  fifo_rd_en;
	wire flow_fifo_empty;
	wire wildcard_entry_vld;

	//wildcard match
	reg [`OPENSKETCH_HASH_INDEX_WIDTH-1:0] dummy_hash_index = {(`OPENSKETCH_HASH_INDEX_WIDTH){1'b0}} + 12'h07f;	
	reg wildcard_wins;
	reg wildcard_loses;
	wire wildcard_match_rdy;

	
	wire                              wildcard_reg_req_out;
	wire                              wildcard_reg_ack_out;
	wire                              wildcard_reg_rd_wr_L_out;
	wire  [`UDP_REG_ADDR_WIDTH-1:0]   wildcard_reg_addr_out;
	wire  [`CPCI_NF2_DATA_WIDTH-1:0]  wildcard_reg_data_out;
	wire  [UDP_REG_SRC_WIDTH-1:0]     wildcard_reg_src_out;
	// Assign Output data
	assign  out_ctrl = in_fifo_ctrl;
	assign  out_data = in_fifo_data;



	// Control to input modules.
	assign in_rdy     =  !in_fifo_nearly_full;
	assign out_wr 	  =  out_wr_int ;

	assign sram_addr = addrfunc_list? (i << countersPerRow + (hash[31:0]) << counterSize) + (hash[31:0])
	:i << countersPerRow + (hash[31:0] ) << counterSize;
	
	// Input fifo
	fallthrough_small_fifo #(
		 .WIDTH(CTRL_WIDTH+DATA_WIDTH),
		 .MAX_DEPTH_BITS(1)
		) input_fifo (
			.din           ({in_ctrl, in_data}),   // Data in
			.wr_en         (in_wr),                // Write enable
			.rd_en         (in_fifo_rd_en),        // Read the next word 
			.dout          ({in_fifo_ctrl, in_fifo_data}),
			.full          (),
			.nearly_full   (in_fifo_nearly_full),
			.empty         (in_fifo_empty),
			.reset         (reset),
			.clk           (clk)
		);



	// Instantiate the Unit Under Test (UUT)
	header_parser 
	#(	.FLOW_ENTRY_SIZE(`OPENSKETCH_ENTRY_WIDTH),
		.ADDITIONAL_WORD_SIZE(16),
		.ADDITIONAL_WORD_BITMASK(16'hefff),
      .ADDITIONAL_WORD_POS(232),
      .ADDITIONAL_WORD_CTRL(8'h40)
		) header_parser
	(
		.in_data(in_data), 
		.in_ctrl(in_ctrl), 
		.in_wr(in_wr), 
		.flow_entry(flow_entry), 
		.flow_entry_src_port(flow_entry_src_port), 
		.pkt_size(pkt_size), 
		.flow_entry_vld(flow_entry_vld), 
		.reset(reset), 
		.clk(clk)
	);
	

	 header_hash
     #(//.INPUT_WIDTH(`OPENSKETCH_ENTRY_WIDTH), 
	  .INPUT_WIDTH(32),  //only src ip for hh now
	  .OUTPUT_WIDTH (32)
	  ) hashing
	 (.data (flow_entry[72+31:72]),
	  .data_ready (flow_entry_vld),
	  .data_vld(wildcard_entry_vld),
	  //.fifo_rd_en(fifo_rd_en),
	  .hash_0 (flow_index_0),
	  .hash_1 (flow_index_1),
	  .hash_2 (flow_index_2),
	  .hash_3 (flow_index_3),
	  .hash_4 (flow_index_4),
	  .hash_5 (flow_index_5),
	  .hash_6 (flow_index_6),
	  .hash_7 (flow_index_7),
	  .hash_8 (flow_index_8),
	  .hash_9 (flow_index_9),
	  .clk (clk),
	  .reset (reset)
	  ); 
	  
	wildcard_match  
	#(.PKT_SIZE_WIDTH(`PKT_SIZE_WIDTH)
     //.UDP_REG_SRC_WIDTH() 
    ) opensketch_wildcard_table
	 (	.flow_entry(dummy_hash_index), 
		//.flow_entry_vld(wildcard_entry_vld), 
		.flow_entry_vld(flow_entry_vld),
		.pkt_size(dout_pkt_size), 
		.wildcard_match_rdy(wildcard_match_rdy), 
		.wildcard_hit(wildcard_hit), 
		.wildcard_miss(wildcard_miss), 
		.wildcard_data(wildcard_data), 
		.wildcard_data_vld(wildcard_data_vld), 
		.wildcard_wins(wildcard_wins), 
		.wildcard_loses(wildcard_loses), 
		.reg_req_in(reg_req_in), 
		.reg_ack_in(reg_ack_in), 
		.reg_rd_wr_L_in(reg_rd_wr_L_in), 
		.reg_addr_in(reg_addr_in), 
		.reg_data_in(reg_data_in), 
		.reg_src_in(reg_src_in), 
		.reg_req_out(wildcard_reg_req_out), 
		.reg_ack_out(wildcard_reg_ack_out), 
		.reg_rd_wr_L_out(wildcard_reg_rd_wr_L_out), 
		.reg_addr_out(wildcard_reg_addr_out), 
		.reg_data_out(wildcard_reg_data_out), 
		.reg_src_out(wildcard_reg_src_out), 
		//.table_flush(table_flush), 
		//.openflow_timer(openflow_timer), 
		.fill(1'b1),
		.reset(reset), 
		.clk(clk)
	);
	
	
	 sram_counter
	 #( .DATA_WIDTH(`DATA_WIDTH),
       .CTRL_WIDTH (`DATA_WIDTH/8)
	 )uu_sram_counter
	(
    .sram_addr(address),
	 .request(wildcard_hit),
	 .func(func_list),
	 .value(constant_value),
	 .nf2_reset(reset),
    .core_clk(clk)
	);
    

  
	always @(*) begin
		if(reset) begin
			in_fifo_rd_en = 0;
			out_wr_int = 0;
		end else begin
			in_fifo_rd_en = 0;
			out_wr_int = 0; 
		  if ((!in_fifo_empty) && out_rdy) begin     
			  out_wr_int = 1;
			  in_fifo_rd_en = 1;
			end
		end 
	end  // always @ (*)
	
	
	
		

	always @(*) begin
		case(hash_idx)
		 32'h0:  hash <= flow_index_0;
		 32'h1:  hash <= flow_index_1;
		 32'h2:  hash <= flow_index_2;
		 32'h3:  hash <= flow_index_3;
		 32'h4:  hash <= flow_index_4;
		 32'h5:  hash <= flow_index_5;
		 32'h6:  hash <= flow_index_6;
		 32'h7:  hash <= flow_index_7;
		 32'h8:  hash <= flow_index_8;
		 32'h9:  hash <= flow_index_9;
		
		 default:  hash <= flow_index_0;
		 endcase
		 

	end
		
	
//---------- Hash and measure -----------
/*

1. Create the custom hash, TCAM ,Memory modules and instantiate it here.
2. Using the FSM above extract suitable fields of the packets required
   and pass it to the hashing and other modules as required.
3. The required entries in user_data_path.v is already done.
   Any changes needs tobe done from this module on.

*/

//---------- End of Hash and measure ---

//Adding Hardware and software registers to the block.
//
generic_regs
   #( 
      .UDP_REG_SRC_WIDTH   (UDP_REG_SRC_WIDTH),
      .TAG                 (`HASHNMEASURE_BLOCK_ADDR),         // Tag -- eg. MODULE_TAG
      .REG_ADDR_WIDTH      (`HASHNMEASURE_REG_ADDR_WIDTH),     // Width of block addresses -- eg. MODULE_REG_ADDR_WIDTH
      .NUM_COUNTERS        (0),               		       // Number of counters
      .NUM_SOFTWARE_REGS   (1),              		       // Number of sw regs
      .NUM_HARDWARE_REGS   (2)                		       // Number of hw regs
   ) module_regs (
      .reg_req_in       (wildcard_reg_req_out),
      .reg_ack_in       (wildcard_reg_ack_out),
      .reg_rd_wr_L_in   (wildcard_reg_rd_wr_L_out),
      .reg_addr_in      (wildcard_reg_addr_out),
      .reg_data_in      (wildcard_reg_data_out),
      .reg_src_in       (wildcard_reg_src_out),

      .reg_req_out      (reg_req_out),
      .reg_ack_out      (reg_ack_out),
      .reg_rd_wr_L_out  (reg_rd_wr_L_out),
      .reg_addr_out     (reg_addr_out),
      .reg_data_out     (reg_data_out),
      .reg_src_out      (reg_src_out),

      // --- counters interface
      .counter_updates  (),
      .counter_decrement(),

      // --- SW regs interface
      .software_regs    ({filed_id, numHashValues, range, 
		numRows,countersPerRow,counterSize, updateType, func_list,addrfunc_list, sram_size,
		rev_filed_id, rev_numHashValues, rev_range,
		numRows2, countersPerRow2, counterSize2, hash_idx2,
		addrfunc_list, hash_idx, i, func_list, constant_value}),

      // --- HW regs interface

      .hardware_regs    (),
      
      .clk              (clk),
      .reset            (reset)
    );



endmodule
