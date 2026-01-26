module ps2_keyboard (
    input wire clk,
    input wire rst_n,
    input wire ps2_dat,
    input wire ps2_clk,
    input wire nextdata_n,

    output wire [7:0] data,
    output reg        ready,
    output reg        overflow,  // fifo overflow
    output reg        f0
);

    reg [9:0] buffer;        // ps2_dat bits
    reg [7:0] fifo[7:0];     // data fifo
    reg [2:0] w_ptr,r_ptr;   // fifo write and read pointers
    reg [3:0] count;         // count ps2_dat bits

    // detect falling edge of ps2_clk
    reg [2:0] ps2_clk_sync;
    always @(posedge clk) begin
        ps2_clk_sync <= {ps2_clk_sync[1:0], ps2_clk};
    end

    wire sampling = ps2_clk_sync[2] & ~ps2_clk_sync[1];

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin // reset
            count <= 0;
            w_ptr <= 0;
            r_ptr <= 0;
            overflow <= 0;
            ready <= 0;
            f0 <= 0;
        end
        else begin
            f0 <= 0;
            if (ready) begin // read to output next data
                if (nextdata_n == 1'b0) begin // read next data
                    r_ptr <= r_ptr + 3'b1;
                    if (w_ptr == (r_ptr + 3'b1)) // empty
                        ready <= 1'b0;
                end
            end

            if (sampling) begin
                if (count == 4'd10) begin
                    if ((buffer[0] == 1'b0) &&  // start bit
                        (ps2_dat == 1'b1)  &&  // stop bit
                        (^buffer[9:1])) begin   // odd parity
                        fifo[w_ptr] <= buffer[8:1];  
                        w_ptr <= w_ptr + 3'b1;
                        ready <= 1'b1;
                        overflow <= overflow | (r_ptr == (w_ptr + 3'b1));
                        f0 <= (buffer[8:1] == 8'hF0);
                    end
                    count <= 0; 
                end else begin
                    buffer[count] <= ps2_dat; // store ps2_dat
                    count <= count + 3'b1;
                end
            end
        end
    end

    assign data = fifo[r_ptr];

endmodule
