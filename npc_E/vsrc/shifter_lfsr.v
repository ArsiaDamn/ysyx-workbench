module shifter_lfsr_N #(
    parameter N = 8
)(
    input clk,
    input rst,
    output reg [N-1:0] out
);
    wire feedback;
    assign feedback = out[0] ^ out[2] ^ out[3] ^ out[4];

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            out <= {{(N-1){1'b0}},1};
        end else begin
            out <= {feedback, out[N-1:1]};
        end
    end
endmodule
