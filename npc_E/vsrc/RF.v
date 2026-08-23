// module RF #(
//     parameter ADDR_WIDTH = 5, 
//     parameter DATA_WIDTH = 32
// ) (

//   input  wire                    clk,
//   input  wire [DATA_WIDTH-1:0]   wdata,
//   input  wire [ADDR_WIDTH-1:0]   rd, 
//   input  wire                    write,
//   input  wire [ADDR_WIDTH-1:0]   rs1,
//   input  wire [ADDR_WIDTH-1:0]   rs2,

//   output wire [DATA_WIDTH-1:0]   RD1,
//   output wire [DATA_WIDTH-1:0]   RD2
// );

//   reg [DATA_WIDTH-1:0] register [0:2**ADDR_WIDTH-1];

//   always @(clk) begin
//         register[0] = 32'h0; 
//     end

//   always @(posedge clk) begin
//     if (write && rd != 0) register[rd] <= wdata;
//   end
  
//   assign RD1 = register[rs1];
//   assign RD2 = register[rs2];

// endmodule
