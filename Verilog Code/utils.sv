// shift register used to store data
module shift_reg #(parameter data_width, length)(
    input clk,  
    input logic [data_width-1:0] data_in,  
    output logic [data_width-1:0] data_out  
);
    //length >= 2
    logic [data_width-1:0] memory [length];
    generate
        genvar i;
        for(i = 0; i < length-1; i++) begin : shift_reg_gen
            always @(posedge clk) begin
                memory[i] <= memory[i+1];
            end
        end
    endgenerate
    always @(posedge clk) begin
            memory[length-1] <= data_in;   
    end
    assign data_out = memory[0]; 
endmodule