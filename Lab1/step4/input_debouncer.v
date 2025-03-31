
/*
   The Input Debouncer (input_debouncer) module ensures that a noisy or bouncing input signal is stabilized for digital
   processing. It samples the input signal and waits for a defined hold time (HOLD_SIGNAL) before confirming the signal's
   stable state, thereby setting the debounced output.

   ?Inputs:
      clk    : The system clock signal
      signal : The input signal to be debounced

   ?Outputs:
      signal_debounced : The stabilized output signal after the input is deemed stable
*/

module input_debouncer #(
    parameter HOLD_SIGNAL = 10_000_000,
    parameter BITS = 24
)(
    input clk, 
    input signal, 
    output reg signal_debounced
);
    reg [2:0] FF_wire;
    reg [BITS-1:0] counter;
    
    always @(posedge clk) begin
        FF_wire[0] <= signal;
        FF_wire[1] <= FF_wire[0];
        FF_wire[2] <= FF_wire[1];
    end

    assign enabled = ~(FF_wire[1] ^ FF_wire[2]);

    always @(posedge clk) begin
        // If counter is HOLD_SIGNAL, then signal_debounced is FF_wire[2]
        if (counter == HOLD_SIGNAL) begin
            signal_debounced <= FF_wire[2];
        end
        // If enabled, increment counter
        if (enabled) begin
            counter <= counter + 1;
        end else begin
            counter <= 0; // Reset counter if not enabled
        end
    end

endmodule