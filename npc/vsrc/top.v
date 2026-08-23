module top(
  input  wire        clk,
  input  wire        rst_n
);

  import "DPI-C" function void npc_trap();
  import "DPI-C" function int  pmem_read(input int raddr);
  import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

  wire [31:0] pc;
  reg  [31:0] inst;
  
  wire        mem_valid;
  wire        mem_wen;
  wire [31:0] mem_addr;
  wire [31:0] mem_wdata;
  wire [3:0]  mem_wmask;
  reg  [31:0] mem_rdata;

  always @(*) begin
    inst = pmem_read(pc); 
    if (mem_valid && !mem_wen) begin
      mem_rdata = pmem_read(mem_addr);
    end else begin
      mem_rdata = 32'b0;
    end
  end

  always @(posedge clk) begin
    if (rst_n == 1'b1) begin
      if (mem_valid && mem_wen) begin
        pmem_write(mem_addr, mem_wdata, {4'b0, mem_wmask});
      end
      if (inst == 32'h00100073) begin
        npc_trap(); 
      end
    end
  end

  riscv u_core (
    .clk(clk),
    .rst(~rst_n),
    .pc (pc),
    .inst(inst),
    .mem_valid(mem_valid),
    .mem_wen  (mem_wen),
    .mem_addr (mem_addr),
    .mem_wdata(mem_wdata),
    .mem_wmask(mem_wmask),
    .mem_rdata(mem_rdata)
    // TODO: mem_valid, mem_wen, mem_addr, mem_wdata, mem_wmask, mem_rdata
  );

endmodule
