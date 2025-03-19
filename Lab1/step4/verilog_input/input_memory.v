module input_memory # (
    parameter INIT_00 = 32'b0,
    parameter INIT_01 = 32'b0,
    parameter INIT_02 = 32'b0,
    parameter INIT_03 = 32'b0,
    parameter INIT_04 = 32'b0,
    parameter INIT_05 = 32'b0,
    parameter INIT_06 = 32'b0,
    parameter INIT_07 = 32'b0
) (
    input clk,
    input reset,
    input scroll_button,
    output wire [7:0] selected_num
);

    reg [31:0] memory [7:0];
    reg [2:0] address;
    assign selected_num = memory[address];

    initial memory[0] = INIT_00;
    initial memory[1] = INIT_01;
    initial memory[2] = INIT_02;
    initial memory[3] = INIT_03;
    initial memory[4] = INIT_04;
    initial memory[5] = INIT_05;
    initial memory[6] = INIT_06;
    initial memory[7] = INIT_07;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            address <= 3'b0;
        end else if (scroll_button) begin
            address <= address + 3'b1;
        end
    end
    
endmodule