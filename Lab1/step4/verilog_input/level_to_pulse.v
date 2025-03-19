module level_to_pulse (
    input clk,
    input reset,
    input level_input,
    output wire pulse_output
);

    parameter IDLE = 2'b00;
    parameter PULSE = 2'b01;
    parameter WAIT = 2'b10;

    reg [1:0] current_state, next_state;

    always @(posedge clk or posedge reset) begin
        if (reset == 1'b1) begin
            current_state <= IDLE;
        end else begin
            current_state <= next_state;
        end
    end

    always @(current_state) begin
        case (current_state)
            IDLE: next_state <= level_input == 1'b1 ? PULSE : IDLE;
            PULSE: next_state <= WAIT;
            WAIT: next_state <= level_input == 1'b0 ? IDLE : WAIT;
            default: next_state <= IDLE;
        endcase
    end

    always @(current_state) begin
        case (current_state)
            IDLE: pulse_output <= 1'b0;
            PULSE: pulse_output <= 1'b1;
            WAIT: pulse_output <= 1'b0; 
            default: pulse_output <= 1'b0;
        endcase
    end
    
endmodule