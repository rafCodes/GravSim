// visitor center used to house the data for the visitor
// this visitor visits all the towns
// receives data from HPS


module visitor_center #(parameter max_m10k_size = 4096, m10k_address_len = 12)(
        input logic clk,
        input logic reset,

        /////////////////////// FROM THE HPS ///////////////////////////

        // values sent to neighborhoods
        input logic [m10k_address_len-1:0] hps_total_number_of_visitors,
        input logic [m10k_address_len-1:0] hps_visitor_index,
        input logic hps_write_enable,

        input [31:0] hps_x,
        input [31:0] hps_y,
        input [31:0] hps_mass,

        /////////////////////// FROM THE HPS ///////////////////////////

        // signal for the visitor center to send the next set of visitors
        input logic next,

        // outputs to towns
        output logic [31:0] visitor_x_pos,
        output logic [31:0] visitor_y_pos,
        output logic [31:0] visitor_mass,
        output logic [m10k_address_len-1:0] visitor_index,

        // last visitor is being sent
        output logic last_visitor
);  

    
    //////////////////////// STATE MACHINE //////////////////////////////
    //indicies and counters

    // index of visitor being sent
    logic [m10k_address_len-1:0] current_visitor;
    
    // states
    logic [2:0] state;
    localparam reset_state = 3'd0;
    localparam hps_fill_state = 3'd1;
    localparam calc_state = 3'd2;
    localparam wait_state = 3'd3;

    //addressing
    wire [m10k_address_len-1:0] current_visitor_address_wire;


    //////////////////////// STATE MACHINE //////////////////////////////

    assign current_visitor_address_wire = current_visitor;

    always @(posedge clk) begin
      if(reset)
        state <= reset_state;
      else begin
        case (state)
            reset_state: begin
                current_visitor <= 0;
                last_visitor <= 0;
                visitor_index <= 0;

                if(hps_write_enable) begin
                    state <= hps_fill_state;
                end

                else begin
                    state <= wait_state;
                end
            end
            
            hps_fill_state: begin
                if(!hps_write_enable) begin
                    // now wait
                    last_visitor <= 0;
                    visitor_index <= 0;
                    current_visitor <= 0;
                    state <= calc_state;
                end

            end

            calc_state: begin
                if (next) begin
                    // increment to next index
                    current_visitor <= current_visitor + 1;

                    // update which the index to the towns
                    visitor_index <= current_visitor + 1;

                end
                if (current_visitor >= hps_total_number_of_visitors - 1) begin
                    //stop when we have gone through all of the visitors
                    // sent to all the towns
                    last_visitor <= 1;

                    // now we wait
                    state <= wait_state;

                end
            end
          
            // wait state
            default: begin
                if(hps_write_enable) begin
                    state <= hps_fill_state;
                end
                else begin
                    state <= wait_state;
                end
            end
        endcase
      end
    end
    
    logic [31:0] full_visitor_x_pos;
    logic [31:0] full_visitor_y_pos;
    logic [31:0] full_visitor_mass;
    assign visitor_x_pos = full_visitor_x_pos;
    assign visitor_y_pos = full_visitor_y_pos;
    assign visitor_mass = full_visitor_mass;
    m10k #(max_m10k_size, m10k_address_len) visitor_x_pos_mem( 
        .clock(clk),
        // reading port for sending to towns
        .address_a(current_visitor_address_wire),
        .data_a('0),
        .q_a(full_visitor_x_pos),
        .wren_a('0),

        // writing port for the hps to write to
        .address_b(hps_visitor_index),
        .data_b(hps_x),
        .q_b(),
        .wren_b(hps_write_enable)
    );

    m10k #(max_m10k_size, m10k_address_len) visitor_y_pos_mem( 
        .clock(clk),
        // reading port for sending to towns
        .address_a(current_visitor_address_wire),
        .data_a('0),
        .q_a(full_visitor_y_pos),
        .wren_a('0),

        // writing port for the hps to write to
        .address_b(hps_visitor_index),
        .data_b(hps_y),
        .q_b(),
        .wren_b(hps_write_enable)
    );

    m10k #(max_m10k_size, m10k_address_len) visitor_mass_mem( 
        .clock(clk),
        // reading port for sending to towns
        .address_a(current_visitor_address_wire),
        .data_a('0),
        .q_a(full_visitor_mass),
        .wren_a('0),

        // writing port for the hps to write to
        .address_b(hps_visitor_index),
        .data_b(hps_mass),
        .q_b(),
        .wren_b(hps_write_enable)
    );
   
endmodule