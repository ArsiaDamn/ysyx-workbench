module add_sub_N #(
    parameter N = 32
)(
    input  wire [N-1:0] A,
    input  wire [N-1:0] B,
    input  wire         op, // 0:add ,1:sub

    output reg  [N-1:0] result,
    output reg          carryout,
    output reg          overflow,          
    output reg          zero
);

    always @(*) begin
        result = 0;
        carryout = 0;
        overflow = 0;
        zero = 0;
        if (op == 1) begin
            {carryout, result} = A + ~B + 1;
            overflow = (A[N-1] != B[N-1]) && (result[N-1] != A[N-1]);
        end else begin
            {carryout, result} = A + B;
            overflow = (A[N-1] == B[N-1]) && (result[N-1] != A[N-1]);
        end
        if (result == 0) 
            zero = 1;
        else 
            zero = 0;
    end
endmodule
