module LZEx6 (
    input [5:0] zrs,
    output reg [2:0] count
);

    always @(zrs) begin
        case (zrs)
            6'bxxxxx0: count = 3'b000;
            6'bxxxx01: count = 3'b001;
            6'bxxx011: count = 3'b010;
            6'bxx0111: count = 3'b011;
            6'bx01111: count = 3'b100;
            6'b011111: count = 3'b101;
            6'b111111: count = 3'b000;
        endcase
    end

endmodule