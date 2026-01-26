module ps2_ASCII(
    input  wire [7:0] ps2_code,
    output wire [7:0] ascii_code
);

    // ASCII码映射表
    function automatic [7:0] lut_normal;
        input [7:0] code;
        begin
            case (code)
                // letters
                8'h1C: lut_normal = "a"; 8'h32: lut_normal = "b"; 8'h21: lut_normal = "c";
                8'h23: lut_normal = "d"; 8'h24: lut_normal = "e"; 8'h2B: lut_normal = "f";
                8'h34: lut_normal = "g"; 8'h33: lut_normal = "h"; 8'h43: lut_normal = "i";
                8'h3B: lut_normal = "j"; 8'h42: lut_normal = "k"; 8'h4B: lut_normal = "l";
                8'h3A: lut_normal = "m"; 8'h31: lut_normal = "n"; 8'h44: lut_normal = "o";
                8'h4D: lut_normal = "p"; 8'h15: lut_normal = "q"; 8'h2D: lut_normal = "r";
                8'h1B: lut_normal = "s"; 8'h2C: lut_normal = "t"; 8'h3C: lut_normal = "u";
                8'h2A: lut_normal = "v"; 8'h1D: lut_normal = "w"; 8'h22: lut_normal = "x";
                8'h35: lut_normal = "y"; 8'h1A: lut_normal = "z";
                // numbers
                8'h45: lut_normal = "0"; 8'h16: lut_normal = "1"; 8'h1E: lut_normal = "2";
                8'h26: lut_normal = "3"; 8'h25: lut_normal = "4"; 8'h2E: lut_normal = "5";
                8'h36: lut_normal = "6"; 8'h3D: lut_normal = "7"; 8'h3E: lut_normal = "8";
                8'h46: lut_normal = "9";
                // others 
                8'h29: lut_normal = " ";     // Space
                8'h5A: lut_normal = 8'h0D;   // Enter
                8'h0E: lut_normal = "`"; 8'h4E: lut_normal = "-"; 8'h55: lut_normal = "=";
                8'h5D: lut_normal = "\\"; 8'h54: lut_normal = "["; 8'h5B: lut_normal = "]";
                8'h4C: lut_normal = ";"; 8'h52: lut_normal = "'"; 8'h41: lut_normal = ",";
                8'h49: lut_normal = "."; 8'h4A: lut_normal = "/";
                default: lut_normal = 8'h00; // 未定义
            endcase
        end
    endfunction

    assign ascii_code = lut_normal(ps2_code);

endmodule
