module top(
  input         clk,      
  input         clrn,     
  input         ps2_clk,  
  input         ps2_data, 
  output [7:0]  seg0,     
  output [7:0]  seg1,
  output [7:0]  seg2,
  output [7:0]  seg3,
  output [7:0]  seg4,
  output [7:0]  seg5
);

  wire [7:0]  scan_code;   
  wire        ready;      
  wire        overflow;   
  reg         nextdata_n; 

  ps2_keyboard ps2_inst(
    .clk        (clk),
    .clrn       (clrn),
    .ps2_clk    (ps2_clk),
    .ps2_data   (ps2_data),
    .nextdata_n(nextdata_n), 
    .data       (scan_code),   
    .ready      (ready),       
    .overflow   (overflow)     
  );

  always @(posedge clk or negedge clrn) begin
      if(!clrn) begin
          nextdata_n <= 1'b1;
      end
      else if(ready) begin
          nextdata_n <= 1'b0;
      end
  end

  wire [7:0]  ascii_code;

  scan2ascii ascii_inst(
    .scan_code(scan_code), 
    .ascii    (ascii_code) 
  );

  reg [7:0] key_count;    
  reg key_release;   
  reg ps2_state;
  reg [7:0] a0;
  reg [7:0] b0;

  always @(posedge clk or negedge clrn) begin
    if (!clrn) begin 
      key_count   <= 8'd0;
      key_release <= 1'b1;    // 松开
      ps2_state   <= 1'd0;    // 空闲，等待数据
      a0 <= 0;
      b0 <= 0;
    end else begin
      if (!ps2_state) begin  //空闲
        if (ready) begin
          if (scan_code == 8'hF0) begin  //等待松开码
            ps2_state <= 1'd1;   
            a0 <= scan_code;
            b0 <= ascii_code;
          end else begin
            key_release <= 1'b0; 
            ps2_state <= 1'd0;
            a0 <= scan_code;
            b0 <= ascii_code;
          end
        end
      end else begin   //等待下一字节
        if (ready) begin   
          key_release <= 1'b1;
          key_count   <= key_count + 8'd1;
          ps2_state   <= 1'd0; 
          a0 <= scan_code;
          b0 <= ascii_code;
        end
      end
    end
  end

  wire [3:0]  cnt_high = key_count[7:4];
  wire [3:0]  cnt_low  = key_count[3:0];

  wire [3:0]  ascii_high = b0[7:4];
  wire [3:0]  ascii_low  = b0[3:0];

  wire [3:0]  scan_high  = a0[7:4];
  wire [3:0]  scan_low   = a0[3:0];

  wire [7:0]  seg0_normal;
  wire [7:0]  seg1_normal;
  wire [7:0]  seg2_normal;
  wire [7:0]  seg3_normal;

  bcd7seg seg0_inst(
    .b(scan_low),   
    .h(seg0_normal)        
  );

  bcd7seg seg1_inst(
    .b(scan_high),  
    .h(seg1_normal)        
  );

  bcd7seg seg2_inst(
    .b(ascii_low),
    .h(seg2_normal)
  );

  bcd7seg seg3_inst(
    .b(ascii_high),
    .h(seg3_normal)
  );

  bcd7seg seg4_inst(
    .b(cnt_low),
    .h(seg4)
  );

  bcd7seg seg5_inst(
    .b(cnt_high),
    .h(seg5)
  );

  assign seg0 = key_release ? 8'hFF : seg0_normal;;
  assign seg1 = key_release ? 8'hFF : seg1_normal;;
  assign seg2 = key_release ? 8'hFF : seg2_normal;;
  assign seg3 = key_release ? 8'hFF : seg3_normal;;

endmodule
