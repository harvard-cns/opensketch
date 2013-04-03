
  module header_hash
    #(parameter INPUT_WIDTH = 32,
      parameter OUTPUT_WIDTH = 19)
    (input [INPUT_WIDTH-1:0]       data,
	  input  							  data_ready,
	  output reg 						  data_vld,
	  //output reg						  fifo_rd_en,
     output reg [OUTPUT_WIDTH-1:0] hash_0,
     output reg [OUTPUT_WIDTH-1:0] hash_1,
	  output reg [OUTPUT_WIDTH-1:0] hash_2,
     output reg [OUTPUT_WIDTH-1:0] hash_3,
	  output reg [OUTPUT_WIDTH-1:0] hash_4,
     output reg [OUTPUT_WIDTH-1:0] hash_5,
	  output reg [OUTPUT_WIDTH-1:0] hash_6,
     output reg [OUTPUT_WIDTH-1:0] hash_7,
	  output reg [OUTPUT_WIDTH-1:0] hash_8,
     output reg [OUTPUT_WIDTH-1:0] hash_9,
	  
     input                         clk,
     input                         reset);

   /* These macros contain the definitions
    * of the hashing functions */
	//`CRC32_D32_0
   //`CRC32_D32_1

   wire [31:0] data_rev;
   wire [31:0] data_padded = {{(32-INPUT_WIDTH){1'b0}}, data};
	reg 		state;
   generate
      genvar 	i;
      for (i=0; i<32; i=i+1) begin: test
	 assign data_rev[(31-i)*8 + 7 : (31-i)*8] = data_padded[i*8 + 7 : i*8];
      end
   endgenerate

   always @( posedge clk  ) begin
		data_vld <= 0;
		if(reset) begin
			hash_0        <= 0;
			hash_1        <= 0;
			hash_2        <= 0;
			hash_3        <= 0;
			hash_4        <= 0;
			hash_5        <= 0;
			hash_6        <= 0;
			hash_7        <= 0;
			hash_8        <= 0;
			hash_9        <= 0;
			data_vld      <= 0;
			state         <= 0;
		end else if (data_ready) begin
			hash_0        <= nextCRC32_D32_0(data_rev, 32'h0);
			hash_1        <= nextCRC32_D32_1(data_rev, 32'h0);
			hash_2        <= nextCRC32_D32_2(data_rev, 32'h0);
			hash_3        <= nextCRC32_D32_3(data_rev, 32'h0);
			hash_4        <= nextCRC32_D32_4(data_rev, 32'h0);
			hash_5        <= nextCRC32_D32_5(data_rev, 32'h0);
			hash_6        <= nextCRC32_D32_6(data_rev, 32'h0);
			hash_7        <= nextCRC32_D32_7(data_rev, 32'h0);
			hash_8        <= nextCRC32_D32_8(data_rev, 32'h0);
			hash_9        <= nextCRC32_D32_9(data_rev, 32'h0);
			
			data_vld <= 1;	
		end 		
   end
endmodule // header_hash

module header_hash_tester ();
   wire [31:0] data;
   wire [31:0] data_rev;
   reg 		clk = 0;

   wire [31:0] 	hash_0_rev;

   always #4 clk = !clk;

   generate
      genvar 	i;
      for (i=0; i<32; i=i+1) begin: test
	 assign data_rev[(31-i)*8 + 7: (31-i)*8] = i + 'h31;
	 assign data[i*8 + 7: i*8] = i + 'h31;
      end
   endgenerate

   header_hash
     #(.INPUT_WIDTH(32), .OUTPUT_WIDTH (32))
       header_hash_rev
	 (.data (data_rev),
	  .hash_0 (hash_0_rev),
	  .hash_1 (),
	  .clk (clk),
	  .reset (1'b0));

endmodule // header_hash_tester
