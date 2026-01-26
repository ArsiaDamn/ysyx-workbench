module mux_241 (
  input  wire [1:0] X0,
  input  wire [1:0] X1,
  input  wire [1:0] X2,
  input  wire [1:0] X3,
  input  wire [1:0] Y,
  output wire [1:0] F
);
 MuxKey #(.NR_KEY(4), .KEY_LEN(2), .DATA_LEN(2)) u_mux (
    .out(F),
    .key(Y),
    .lut({
      2'b11, X3,   // pair_list[3]
      2'b10, X2,   // pair_list[2]
      2'b01, X1,   // pair_list[1]
      2'b00, X0    // pair_list[0] (lowest bits)
    })
  );

endmodule
