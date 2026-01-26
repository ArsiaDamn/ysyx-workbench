module ALU_N #(
    parameter N = 32
)(
    input [N-1:0]A,
    input [N-1:0]B,
    input [2:0]opcode,
    output reg [N-1:0]out,
    output reg zero,
    output reg carryout,
    output reg overflow
);

    reg [N-1:0] sum_temp;
    reg cout_temp;
    always @(*) begin
        //initial
        out = 0;
        zero = 0;
        carryout = 0;
        overflow = 0;
        sum_temp = 0;
        cout_temp = 0;
        case (opcode)
            3'b000: begin // add
                {carryout, out} = A + B;
                overflow = (A[N-1] == B[N-1]) && (out[N-1] != A[N-1]);
            end
            3'b001: begin // sub
                {carryout, out} = A + (~B) + 1;
                overflow = (A[N-1] != B[N-1]) && (out[N-1] != A[N-1]);
            end
            3'b010: begin // not
                out = ~A;
            end
            3'b011: begin // and
                out = A & B;
            end
            3'b100: begin // or
                out = A | B;
            end
            3'b101: begin // xor
                out = A ^ B;
            end
            3'b110: begin // A<B
                {cout_temp, sum_temp} = A + (~B) + 1'b1;
                overflow = (A[N-1] != B[N-1]) && (sum_temp[N-1] != A[N-1]);
                if (sum_temp[N-1] ^ overflow)  out = {{N-1{1'b0}}, 1'b1}; 
                else                           out = {N{1'b0}};     
            end
            3'b111: begin // A==B
                if (A == B) out = {{N-1{1'b0}}, 1'b1};
                else        out = {N{1'b0}};
            end
            default: begin
                out = 0;
            end
        endcase

        if (out == 0) begin
            zero = 1;
        end else begin
            zero = 0;
        end
    end
endmodule
