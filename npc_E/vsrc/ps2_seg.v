module ps2_seg7(
  input  wire [3:0] y,     
  output reg  [7:0] LED
);

  always @(*) begin
    case(y)
    0:LED = 8'b00000011;
    1:LED = 8'b10011111;
    2:LED = 8'b00100101;
    3:LED = 8'b00001101;
    4:LED = 8'b10011001;
    5:LED = 8'b01001001;
    6:LED = 8'b01000001;
    7:LED = 8'b00011111;
    8:LED = 8'b00000001;
    9:LED = 8'b00001001;
    10:LED = 8'b00010001;
    11:LED = 8'b11000001;
    12:LED = 8'b01100011;
    13:LED = 8'b10000101;
    14:LED = 8'b01100001;
    15:LED = 8'b01110001;
    default:LED = 8'b11111111;
    endcase 
  end
endmodule