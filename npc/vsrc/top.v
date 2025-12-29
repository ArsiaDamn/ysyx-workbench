module top(
    input  wire clk,
    input  wire rst_n,
    input  wire ps2_clk,
    input  wire ps2_dat,

    output wire [7:0] seg7,
    output wire [7:0] seg6,
    output wire [7:0] seg5,
    output wire [7:0] seg4,
    output wire [7:0] seg3,
    output wire [7:0] seg2,
    output wire [7:0] seg1,
    output wire [7:0] seg0
);

    // PS/2
    wire [7:0] kb_data;
    wire       kb_ready;
    wire       kb_overflow;
    reg        nextdata_n;
    wire       f0;            

    ps2_keyboard u_kb(
        .clk(clk),
        .rst_n(rst_n),
        .ps2_dat(ps2_dat),
        .ps2_clk(ps2_clk),
        .nextdata_n(nextdata_n),
        .data(kb_data),
        .ready(kb_ready),
        .overflow(kb_overflow),
        .f0(f0)
    );

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n) nextdata_n <= 1;
        if(kb_ready) nextdata_n <= 0;
    end


    // ASCII
    wire [7:0] ascii_code;

    ps2_ASCII u_ascii(
        .ps2_code(kb_data),
        .ascii_code(ascii_code)
    );

    // 显示计数
    reg [7:0] last_scan;
    reg [7:0] last_ascii;
    reg [7:0] press_count;
    reg       unpress;

    // FSM 
    reg current_state, next_state;
    localparam srelease = 1'b0;  // 0:松
    localparam spress   = 1'b1;  // 1:按 (F0等断码)

    // 一段
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) 
            current_state <= srelease;
        else 
            current_state <= next_state;
    end

    // 二段
    always @(*) begin
        next_state = current_state;
        case (current_state)
            srelease: begin
                if (kb_ready) begin
                    if (kb_data == 8'hF0) 
                        next_state = spress;
                    else 
                        next_state = srelease;
                end
            end
            spress: begin
                if (kb_ready) 
                    next_state = srelease;
            end
            default: next_state = srelease;
        endcase
    end

    // 三段
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin 
            press_count <= 0;
            unpress     <= 1;
            last_scan   <= 0;
            last_ascii  <= 0;
        end else begin
            case (current_state) 
                srelease: begin
                    if (kb_ready) begin
                        if (kb_data != 8'hF0) begin
                            unpress     <= 0;
                            last_scan   <= kb_data;
                            last_ascii  <= ascii_code;
                        end else begin // F0               
                            last_scan   <= kb_data;
                            last_ascii  <= ascii_code;
                        end
                    end
                end
                spress: begin
                    if (kb_ready) begin
                        unpress     <= 1;
                        press_count <= press_count + 1;
                    end
                end
            endcase
        end
    end

    // // 显示计数
    // reg [7:0] last_scan;
    // reg [7:0] last_ascii;
    // reg [7:0] press_count;
    // reg       unpress;
    // reg       ps2_state;  // 0:松 1:按

    // always @(posedge clk or negedge rst_n) begin
    //     if (!rst_n) begin 
    //         press_count <= 0;
    //         unpress <= 1;
    //         ps2_state   <= 0;   
    //         last_scan   <= 0;
    //         last_ascii  <= 0;
    //     end else begin
    //         if (!ps2_state) begin
    //             if (kb_ready) begin
    //                 if (kb_data == 8'hF0) begin
    //                     ps2_state  <= 1;
    //                     last_scan  <= kb_data;
    //                     last_ascii <= ascii_code;
    //                 end else begin
    //                     unpress <= 0;
    //                     ps2_state   <= 0;
    //                     last_scan   <= kb_data;
    //                     last_ascii  <= ascii_code;
    //                 end
    //             end
    //         end else begin 
    //             if (kb_ready) begin
    //                 unpress <= 1;
    //                 press_count <= press_count + 1;
    //                 ps2_state   <= 0;
    //             end
    //         end
    //     end
    // end

 
    // 7段译码
    localparam [7:0] black = 8'hFF;

    // 计数
    wire [7:0] seg_cnt_hi, seg_cnt_lo;
    ps2_seg7 u_cnt_hi(.y(press_count[7:4]), .LED(seg_cnt_hi));
    ps2_seg7 u_cnt_lo(.y(press_count[3:0]), .LED(seg_cnt_lo));

    // ASCII
    wire [7:0] seg_ascii_hi, seg_ascii_lo;
    ps2_seg7 u_ascii_hi(.y(last_ascii[7:4]), .LED(seg_ascii_hi));
    ps2_seg7 u_ascii_lo(.y(last_ascii[3:0]), .LED(seg_ascii_lo));

    // 键码
    wire [7:0] seg_scan_hi, seg_scan_lo;
    ps2_seg7 u_scan_hi(.y(last_scan[7:4]), .LED(seg_scan_hi));
    ps2_seg7 u_scan_lo(.y(last_scan[3:0]), .LED(seg_scan_lo));

    assign seg7 = black;
    assign seg6 = black;
    assign seg5 = seg_cnt_hi;
    assign seg4 = seg_cnt_lo;

    assign seg3 = unpress ? black : seg_ascii_hi;
    assign seg2 = unpress ? black : seg_ascii_lo;
    assign seg1 = unpress ? black : seg_scan_hi;
    assign seg0 = unpress ? black : seg_scan_lo;

endmodule
