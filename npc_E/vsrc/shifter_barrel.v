module shifter_barrel #(
     parameter N = 32                  //Must be power of 2
)(
    input wire [N-1:0] d_in,
    input wire [$clog2(N)-1:0] shamt, 
    input wire LR,                    // 1: Left, 0: Right
    input wire AL,                    // 1: Arithmetic, 0: Logical
    output reg [N-1:0] q_out
 );
    always @(*) begin
        if (LR == 1'b1) begin
            q_out = d_in << shamt;  // Left Shift
        end else begin
            if (AL == 1'b1) begin
                q_out = $signed(d_in) >>> shamt; // Arithmetic Right Shift
            end else begin
                q_out = d_in >> shamt; // Logical Right Shift
            end
        end
    end
endmodule
