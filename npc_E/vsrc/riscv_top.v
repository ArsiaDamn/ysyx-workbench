// module top(
//   input  wire        clk,
//   input  wire        rst_n,

//   output wire [31:0] pc,
//   input  wire [31:0] inst,

//   output wire        mem_valid,
//   output wire        mem_wen,
//   output wire [31:0] mem_addr,
//   output wire [31:0] mem_wdata,
//   output wire [3:0]  mem_wmask,
//   input  wire [31:0] mem_rdata
// );

//   reg [31:0] pc_r;
//   assign pc = pc_r;

//   assign mem_valid = 1'b0;
//   assign mem_wen   = 1'b0;
//   assign mem_addr  = 32'b0;
//   assign mem_wdata = 32'b0;
//   assign mem_wmask = 4'b0;

//   wire [6:0] opcode = inst[6:0];
//   wire [2:0] funct3 = inst[14:12];
//   wire [4:0] rs1 = inst[19:15];
//   wire [4:0] rs2 = inst[24:20];
//   wire [4:0] rd  = inst[11:7];

//   wire [31:0] immI = {{20{inst[31]}}, inst[31:20]};

//   wire [31:0] RD1;
//   wire [31:0] RD2;

//   wire [31:0] addi_res = RD1 + immI;

//   RF #(
//     .ADDR_WIDTH(5),
//     .DATA_WIDTH(32)
//   ) rf (
//     .clk  (clk),
//     .wdata(addi_res),
//     .rd   (rd),
//     // .write(is_addi),
//     .rs1  (rs1),
//     .rs2  (rs2),
//     .RD1  (RD1),
//     .RD2  (RD2)
//   );

//   always @(posedge clk) begin
//     if (!rst_n) begin
//       pc_r <= 32'h0;
//     end else begin
//       pc_r <= pc_r + 32'd4;
//     end
//   end

// endmodule