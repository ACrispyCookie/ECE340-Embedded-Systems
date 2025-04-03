
/*
   The Input Memory (input_memory) module stores eight 32-bit values (provided through the INIT_XX parameters) and cycles
   through them each time the scroll_button is pressed. The currently addressed value is provided on selected_num.

   ?Inputs:
      clk          : Clock signal
      reset        : Reset signal
      scroll_button: Increments the address pointer to select the next memory location

   ?Outputs:
      selected_num : 32-bit data output from the currently addressed memory location
*/

module input_memory # (
    parameter INIT_00 = 32'b0,
    parameter INIT_01 = 32'b0,
    parameter INIT_02 = 32'b0,
    parameter INIT_03 = 32'b0,
    parameter INIT_04 = 32'b0,
    parameter INIT_05 = 32'b0,
    parameter INIT_06 = 32'b0,
    parameter INIT_07 = 32'b0,
    parameter INIT_08 = 32'b0,
    parameter INIT_09 = 32'b0
) (
    input clk,
    input reset,
    input scroll_button,
    output wire [31:0] selected_num
);

    reg [31:0] memory [9:0];
    reg [3:0] address;
    assign selected_num = memory[address];

    initial memory[0] = INIT_00;
    initial memory[1] = INIT_01;
    initial memory[2] = INIT_02;
    initial memory[3] = INIT_03;
    initial memory[4] = INIT_04;
    initial memory[5] = INIT_05;
    initial memory[6] = INIT_06;
    initial memory[7] = INIT_07;
    initial memory[8] = INIT_08;
    initial memory[9] = INIT_09;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            address <= 4'b0;
        end else if (scroll_button) begin
            if (address == 4'd9) 
                address <= 4'd0;
            else
                address <= address + 4'b1;
        end
    end
    
endmodule