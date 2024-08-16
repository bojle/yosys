(* techmap_celltype = "$mul $__mul" *)
module \$mul (Y, A, B);
  parameter A_SIGNED = 1;
  parameter A_WIDTH = 18;
  parameter B_SIGNED = 1;
  parameter B_WIDTH = 18;
  parameter Y_WIDTH = 36;

  localparam EFX_MULT_WIDTH = 18;

  input  signed [A_WIDTH-1:0] A;
  input signed [B_WIDTH-1:0] B;
  output signed [Y_WIDTH-1:0] Y;

  wire _TECHMAP_FAIL_ = A_WIDTH > 18 || B_WIDTH > 18 || 
                        (A_WIDTH != B_WIDTH) || (A_SIGNED != B_SIGNED);

  localparam A_PAD_WIDTH = EFX_MULT_WIDTH - A_WIDTH;
  localparam B_PAD_WIDTH = EFX_MULT_WIDTH - B_WIDTH;
  localparam Y_PAD_WIDTH = EFX_MULT_WIDTH * 2 - Y_WIDTH;

  wire signed [Y_PAD_WIDTH-1:0] open_o;
  wire signed [EFX_MULT_WIDTH-1:0] a_in = {{A_PAD_WIDTH{1'b0}},{A}};
  wire signed [EFX_MULT_WIDTH-1:0] b_in = {{B_PAD_WIDTH{1'b0}},{B}};

  EFX_MULT #(
    .WIDTH(EFX_MULT_WIDTH),
    .A_REG(1'b0),
    .B_REG(1'b0),
    .O_REG(1'b0),
    .CLK_POLARITY(1'b1), // 0 falling edge, 1 rising edge
    .CEA_POLARITY(1'b1), // 0 negative, 1 positive
    .RSTA_POLARITY(1'b1), // 0 negative, 1 positive
    .RSTA_SYNC(1'b0), // 0 async, 1 sync
    .RSTA_VALUE(1'b0), // 0 reset, 1 set
    .CEB_POLARITY(1'b1), // 0 negative, 1 positive
    .RSTB_POLARITY(1'b1), // 0 negative, 1 positive
    .RSTB_SYNC(1'b0), // 0 async, 1 sync
    .RSTB_VALUE(1'b0), // 0 reset, 1 set
    .CEO_POLARITY(1'b1), // 0 negative, 1 positive
    .RSTO_POLARITY(1'b1), // 0 negative, 1 positive
    .RSTO_SYNC(1'b0), // 0 async, 1 sync
    .RSTO_VALUE(1'b0), // 0 reset, 1 set
    .SR_SYNC_PRIORITY(1'b1), // 0 CE, 1 SR
  ) _TECHMAP_REPLACE_ (
    .A(a_in),
    .B(b_in),
    .O({open_o, Y}),
    .CLK(1'b0),
    .CEA(1'b0),
    .RSTA(1'b0),
    .CEB(1'b0),
    .RSTB(1'b0),
    .CEO(1'b0),
    .RSTO(1'b0),
  );
endmodule
