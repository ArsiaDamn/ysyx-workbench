module top(
  input  wire        clk,
  input  wire        rst_n,
  input  wire [31:0] inst,
  input  wire [31:0] mem_rdata,

  output wire [31:0] pc,
  output wire        mem_valid,
  output wire        mem_wen,
  output wire [31:0] mem_addr,
  output wire [31:0] mem_wdata,
  output wire [3:0]  mem_wmask
  
);

  riscv u_core (
    .clk(clk),
    .rst(~rst_n),
    .pc (pc),
    .inst(inst)
  );


  assign mem_valid = 1'b0;
  assign mem_wen   = 1'b0;
  assign mem_addr  = 32'b0;
  assign mem_wdata = 32'b0;
  assign mem_wmask = 4'b0;

endmodule

