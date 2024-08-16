module EFX_LUT4(
   output O, 
   input I0,
   input I1,
   input I2,
   input I3
);
	parameter LUTMASK = 16'h0000;

	wire [7:0] s3 = I3 ? LUTMASK[15:8] : LUTMASK[7:0];
	wire [3:0] s2 = I2 ?      s3[ 7:4] :      s3[3:0];
	wire [1:0] s1 = I1 ?      s2[ 3:2] :      s2[1:0];
	assign O = I0 ? s1[1] : s1[0];	   
endmodule

module EFX_ADD(
   output O,
   output CO,
   input I0,
   input I1,
   input CI
);
   parameter I0_POLARITY   = 1;
   parameter I1_POLARITY   = 1;

   wire i0;
   wire i1;

   assign i0 = I0_POLARITY ? I0 : ~I0;
   assign i1 = I1_POLARITY ? I1 : ~I1;

   assign {CO, O} = i0 + i1 + CI;
endmodule

module EFX_FF(
   output reg Q,
   input D,
   input CE,
   (* clkbuf_sink *)
   input CLK,
   input SR
);
   parameter CLK_POLARITY = 1;
   parameter CE_POLARITY = 1;
   parameter SR_POLARITY = 1;
   parameter SR_SYNC = 0;
   parameter SR_VALUE = 0;
   parameter SR_SYNC_PRIORITY = 0;
   parameter D_POLARITY = 1;

   wire clk;
   wire ce;
   wire sr;
   wire d;
   wire prio;
   wire sync;
   wire async;

   assign clk = CLK_POLARITY ? CLK : ~CLK;
   assign ce = CE_POLARITY ? CE : ~CE;
   assign sr = SR_POLARITY ? SR : ~SR;
   assign d = D_POLARITY ? D : ~D;

	initial Q = 1'b0;

   generate
   	if (SR_SYNC == 1) 
      begin
         if (SR_SYNC_PRIORITY == 1) 
         begin
            always @(posedge clk)
               if (sr)
                  Q <= SR_VALUE;
               else if (ce)
                  Q <= d;
         end
         else
         begin
            always @(posedge clk)
               if (ce)
               begin
                  if (sr)
                     Q <= SR_VALUE;
                  else
                     Q <= d;
               end
         end
      end
      else
      begin
         always @(posedge clk or posedge sr)
            if (sr)
               Q <= SR_VALUE;
            else if (ce)
               Q <= d;
         
      end
   endgenerate
endmodule

module EFX_GBUFCE(
   input CE,
   input I,
   (* clkbuf_driver *)
   output O
);
   parameter CE_POLARITY = 1'b1;

   wire ce;
   assign ce = CE_POLARITY ? CE : ~CE;
   
   assign O = I & ce;
   
endmodule

module EFX_RAM_5K
# (
   parameter READ_WIDTH = 20,
   parameter WRITE_WIDTH = 20,
   localparam READ_ADDR_WIDTH = 
			    (READ_WIDTH == 16) ? 8 :  // 256x16
			    (READ_WIDTH == 8)  ? 9 :  // 512x8
			    (READ_WIDTH == 4)  ? 10 : // 1024x4
			    (READ_WIDTH == 2)  ? 11 : // 2048x2
			    (READ_WIDTH == 1)  ? 12 : // 4096x1
			    (READ_WIDTH == 20) ? 8 :  // 256x20
			    (READ_WIDTH == 10) ? 9 :  // 512x10
			    (READ_WIDTH == 5)  ? 10 : -1, // 1024x5
   
   localparam WRITE_ADDR_WIDTH = 
			    (WRITE_WIDTH == 16) ? 8 :  // 256x16
			    (WRITE_WIDTH == 8)  ? 9 :  // 512x8
			    (WRITE_WIDTH == 4)  ? 10 : // 1024x4
			    (WRITE_WIDTH == 2)  ? 11 : // 2048x2
			    (WRITE_WIDTH == 1)  ? 12 : // 4096x1
			    (WRITE_WIDTH == 20) ? 8 :  // 256x20
			    (WRITE_WIDTH == 10) ? 9 :  // 512x10
			    (WRITE_WIDTH == 5)  ? 10 : -1 // 1024x5
)
(
   input [WRITE_WIDTH-1:0] WDATA,
   input [WRITE_ADDR_WIDTH-1:0] WADDR,
   input WE, 
   (* clkbuf_sink *)
   input WCLK,
   input WCLKE, 
   output [READ_WIDTH-1:0] RDATA, 
   input [READ_ADDR_WIDTH-1:0] RADDR,
   input RE, 
   (* clkbuf_sink *)
   input RCLK
);
   parameter OUTPUT_REG = 1'b0;
   parameter RCLK_POLARITY  = 1'b1;
   parameter RE_POLARITY    = 1'b1;
   parameter WCLK_POLARITY  = 1'b1;
   parameter WE_POLARITY    = 1'b1;
   parameter WCLKE_POLARITY = 1'b1;
   parameter WRITE_MODE = "READ_FIRST";
   parameter INIT_0 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_1 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_2 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_3 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_4 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_5 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_6 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_7 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_8 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_9 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_A = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_B = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_C = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_D = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_E = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_F = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_10 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_11 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_12 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
   parameter INIT_13 = 256'h0000000000000000000000000000000000000000000000000000000000000000;
endmodule
`define STRICT_MULT_CHECK 1

module EFX_MULT
(
   A, B, O, CLK, CEA, RSTA, CEB, RSTB, CEO, RSTO
);
   
   parameter WIDTH        = 18;
   parameter A_REG        = 1'b0;
   parameter B_REG        = 1'b0;
   parameter O_REG        = 1'b0;
   parameter CLK_POLARITY = 1'b1; // 0 falling edge, 1 rising edge
   parameter CEA_POLARITY = 1'b1; // 0 negative, 1 positive
   parameter RSTA_POLARITY = 1'b1; // 0 negative, 1 positive
   parameter RSTA_SYNC     = 1'b0; // 0 async, 1 sync
   parameter RSTA_VALUE    = 1'b0; // 0 reset, 1 set
   parameter CEB_POLARITY = 1'b1; // 0 negative, 1 positive
   parameter RSTB_POLARITY = 1'b1; // 0 negative, 1 positive
   parameter RSTB_SYNC     = 1'b0; // 0 async, 1 sync
   parameter RSTB_VALUE    = 1'b0; // 0 reset, 1 set
   parameter CEO_POLARITY = 1'b1; // 0 negative, 1 positive
   parameter RSTO_POLARITY = 1'b1; // 0 negative, 1 positive
   parameter RSTO_SYNC     = 1'b0; // 0 async, 1 sync
   parameter RSTO_VALUE    = 1'b0; // 0 reset, 1 set
   parameter SR_SYNC_PRIORITY = 1'b1; // 0 CE, 1 SR
   
   initial begin
	  // Check for illegal width
	  if (`STRICT_MULT_CHECK) begin
		 if (WIDTH != 16 && WIDTH != 18 ) begin
			$display("ERROR:Illegal WIDTH %d", WIDTH);
			$finish();
		 end
	  end
   end

   localparam IN_DATA_WIDTH = WIDTH;
   localparam OUT_DATA_WIDTH = WIDTH*2;

   input  signed [IN_DATA_WIDTH-1:0] A, B;
   output signed [OUT_DATA_WIDTH-1:0] O;
   input CLK, CEA, RSTA, CEB, RSTB, CEO, RSTO;

   wire  signed [IN_DATA_WIDTH-1:0] A_ff, B_ff, A_int, B_int;
   wire  signed [OUT_DATA_WIDTH-1:0] O_ff, O_int;

   // Optional input registers
   genvar i;
   generate for(i=0;i<IN_DATA_WIDTH;i=i+1) begin : inreg
	  INIT_MULT_FF #(.CLK_POLARITY(CLK_POLARITY), .CE_POLARITY(CEA_POLARITY), .SR_POLARITY(RSTA_POLARITY), .SR_VALUE(RSTA_VALUE), .SR_SYNC(RSTA_SYNC), .SR_SYNC_PRIORITY(SR_SYNC_PRIORITY)) 
	      ffa (.D(A[i]), .CLK(CLK), .CE(CEA), .SR(RSTA), .Q(A_ff[i]));
	  
	  INIT_MULT_FF #(.CLK_POLARITY(CLK_POLARITY), .CE_POLARITY(CEB_POLARITY), .SR_POLARITY(RSTB_POLARITY), .SR_VALUE(RSTB_VALUE), .SR_SYNC(RSTB_SYNC), .SR_SYNC_PRIORITY(SR_SYNC_PRIORITY)) 
	      ffb(.D(B[i]), .CLK(CLK), .CE(CEB), .SR(RSTB), .Q(B_ff[i]));
   end
   endgenerate

   assign A_int = (A_REG == 1) ? A_ff : A;
   assign B_int = (B_REG == 1) ? B_ff : B;
   
   assign O_int = A_int * B_int;

   // Optional output registers
   genvar j;
   generate for(j=0;j<OUT_DATA_WIDTH;j=j+1) begin : outreg
	  INIT_MULT_FF #(.CLK_POLARITY(CLK_POLARITY), .CE_POLARITY(CEO_POLARITY), .SR_POLARITY(RSTO_POLARITY), .SR_VALUE(RSTO_VALUE), .SR_SYNC(RSTO_SYNC), .SR_SYNC_PRIORITY(SR_SYNC_PRIORITY)) 
	      ffo(.D(O_int[j]), .CLK(CLK), .CE(CEO), .SR(RSTO), .Q(O_ff[j]));
   end
   endgenerate

   assign O = (O_REG == 1) ? O_ff : O_int;

endmodule // EFX_MULT

module INIT_MULT_FF # 
(
 parameter CLK_POLARITY = 1'b1, // 0 falling edge, 1 rising edge
 parameter CE_POLARITY  = 1'b1, // 0 negative, 1 positive
 parameter SR_POLARITY  = 1'b1, // 0 negative, 1 positive
 parameter SR_SYNC      = 1'b0, // 0 async, 1 sync
 parameter SR_VALUE     = 1'b0, // 0 reset, 1 set
 parameter SR_SYNC_PRIORITY = 1'b1, // 0 CE, 1 SR
 parameter D_POLARITY   = 1'b1  // 0 invert
)
(
 input 	    D, // data input
 input 	    CE, // clock-enable
 input 	    CLK, // clock
 input 	    SR, // asyc/sync set/reset
 output reg Q = 1'b0    // data output
);
   // Create nets for optional control inputs
   // allows us to assign to them without getting warning
   // for coercing input to inout
   wire     CE_net;
   wire     SR_net;

   // Default values for optional control signals
   assign  CE_net = CE_POLARITY ? 1'b1 : 1'b0;
   assign  SR_net = SR_POLARITY ? 1'b0 : 1'b1;

   // Now assign the input
   assign CE_net = CE;
   assign SR_net = SR;
   
   // Internal signals
   wire d_int;
   wire ce_int;
   wire clk_int;
   wire sr_int;
   wire sync_sr_int;
   wire async_sr_int;
   wire priority_ce_int;
     
   // Check parameters and set internal signals appropriately
   
   // Check clock polarity
   assign clk_int = CLK_POLARITY ? CLK : ~CLK;
   
   // Check clock-enable polarity
   assign ce_int = CE_POLARITY ? CE_net : ~CE_net;

   // Check set/reset polarity
   assign sr_int = SR_POLARITY ? SR_net : ~SR_net;

   // Check datas polarity
   assign d_int = D_POLARITY ? D : ~D;

   // Decide if set/reset is sync or async
   assign sync_sr_int = SR_SYNC ? sr_int : 1'b0;
   assign async_sr_int = SR_SYNC ? 1'b0 : sr_int;

   // Decide if CE or sync SR is a priority
   assign priority_ce_int = SR_SYNC_PRIORITY ? 1'b1 : ce_int;

   // Actual FF guts, everything is positive logic
   always @(posedge async_sr_int or posedge clk_int)
     // Only one of async/sync sr will be valid
     if (async_sr_int)
       Q <= SR_VALUE;
     else if (priority_ce_int)
       if (sync_sr_int)
		 Q <= SR_VALUE;
	   else if (ce_int)
		 Q <= d_int;

endmodule // INIT_MULT_FF

