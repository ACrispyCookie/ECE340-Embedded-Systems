
module LZEx6 (
    input [5:0] zrs,
    output [2:0] count
);
    assign count[0] = (zrs[0] & ~zrs[1]) | (zrs[0] & zrs[2] & (~zrs[3] | (zrs[4] & ~zrs[5])));
    assign count[1] = (zrs[0] & zrs[1] & (~zrs[2] | ~zrs[3])) | (zrs[0] & zrs[1] & zrs[2] & zrs[3] & zrs[4] & zrs[5]);
    assign count[2] = (zrs[0] & zrs[1] & zrs[2] & zrs[3]) | (zrs[0] & zrs[1] & zrs[2] & zrs[3] & zrs[4]);
endmodule