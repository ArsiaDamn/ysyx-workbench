module shifter_N #(
    parameter N = 8
)(
    input clk,
    input rst_n,
    input [2:0] op,
    input [N-1:0] d_in,
    input wire s_in,    // 串行输入
    output reg [N-1:0] q_out
);
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      q_out <= {N{1'b0}};
    end else begin
      case (op)
        3'b000: q_out <= {N{1'b0}};                 // clear
        3'b001: q_out <= d_in;                      // load
        3'b010: q_out <= {1'b0,q_out[N-1:1]};       // logical shift right
        3'b011: q_out <= {q_out[N-2:0], 1'b0};      // logical shift left
        3'b100: q_out <= {q_out[N-1],q_out[N-1:1]}; // arithmetic shift right
        3'b101: q_out <= {s_in, q_out[N-1:1]};      // shift right with serial in
        3'b110: q_out <= {q_out[0], q_out[N-1:1]};  // rotate right
        3'b111: q_out <= {q_out[N-2:0], q_out[N-1]};// rotate left
        default: q_out <= q_out;                    // hold
      endcase
    end
  end
endmodule
