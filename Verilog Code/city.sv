/**
* Top level module that creates multiple gravity acceleration solvers and feeds data to them
*
*/
module city #(parameter max_size = 4096, m10k_address_len = 12, neighborhood_count = 1) (
    input logic clk,
    input logic reset,

    input logic sending,

    input logic [m10k_address_len-1:0] num_objects,

    input logic [m10k_address_len-1:0] pos_write_addr,
    input logic pos_we,
    input logic [31:0] x_pos_write_data_in,
    input logic [31:0] y_pos_write_data_in,
    input logic [31:0] mass_write_data_in,

    input logic [m10k_address_len-1:0] extern_accel_addr,
    output logic [31:0] x_accel_out,
    output logic [31:0] y_accel_out,

    output logic done
);   
    //Define how big the submodules m10k blocks are
    localparam max_neighborhood_m10k_size = (max_size / neighborhood_count)+1;
    //Define how long the submodules m10k address length is (for sending data to them as well)
    localparam neighborhood_m10k_address_len = m10k_address_len - neighborhood_count/2;
       
    //When does each neighborhood want the next visitor
    logic next[neighborhood_count];
    //When is each neighborhood done
    logic [neighborhood_count-1:0] local_done;
    //When all neighborhoods done, send done to HPS
    assign done = (&local_done);
    //If this is the last visitor
    logic last_visitor;
    //Visitor data signals
    logic [31:0] visitor_x_pos;
    logic [31:0] visitor_y_pos;
    logic [31:0] visitor_mass;
    //Visitor index from visitor center
    logic [m10k_address_len-1:0] visitor_index;


    visitor_center visitor_generator(
        .clk(clk),
        .reset(reset),
        .hps_total_number_of_visitors(num_objects),
        .hps_visitor_index(pos_write_addr),
        .hps_write_enable(pos_we),
        .hps_x(x_pos_write_data_in),
        .hps_y(y_pos_write_data_in),
        .hps_mass(mass_write_data_in),
        .next(next[0]),
        .visitor_x_pos(visitor_x_pos),
        .visitor_y_pos(visitor_y_pos),
        .visitor_mass(visitor_mass),
        .visitor_index(visitor_index),
        .last_visitor(last_visitor)
    );
    //Find max index for neighborhoods
    logic [neighborhood_m10k_address_len-1:0] neighborhood_max_index;
    assign neighborhood_max_index = (num_objects%neighborhood_count == 0) ? (num_objects/neighborhood_count)-'d1 : num_objects/neighborhood_count;
    
    //Convert from global indexing to local indexing 
    logic [neighborhood_m10k_address_len-1:0] converted_visitor_index;
    logic [neighborhood_m10k_address_len-1:0] converted_hps_index;
    logic [neighborhood_m10k_address_len-1:0] converted_accel_index;
    assign converted_visitor_index = visitor_index/neighborhood_count;
    assign converted_hps_index = pos_write_addr/neighborhood_count;
    assign converted_accel_index = extern_accel_addr/neighborhood_count;
    
    //Select signals so that only 1 neighborhood 0's mass for each converted index instead of all of the neighborhoods
    logic hps_index_valid[neighborhood_count];
    logic relative_visitor_valid[neighborhood_count];
    logic accel_select[neighborhood_count];
    always_comb begin
        int i;
        for(i = 0; i < neighborhood_count; i++) begin
            // hps_index_valid[i] = (pos_write_addr >= neighborhood_max_index*i  && pos_write_addr < neighborhood_max_index*(i+1));
            // relative_visitor_valid[i] = (visitor_index >= neighborhood_max_index*i  && visitor_index < neighborhood_max_index*(i+1));
            hps_index_valid[i] = (pos_write_addr % neighborhood_count) == i;
            relative_visitor_valid[i] = (visitor_index % neighborhood_count) == i;
        end
    end
    
    //Mux out the acceleration data so that if index is x mod 0 read from neighborhood 0 where the object is stored
    logic [31:0] local_x_accel_out[neighborhood_count];
    logic [31:0] local_y_accel_out[neighborhood_count];
    assign x_accel_out = local_x_accel_out[extern_accel_addr%neighborhood_count];
    assign y_accel_out = local_y_accel_out[extern_accel_addr%neighborhood_count];

    //Generate neighborhoods
    generate
        genvar i;
        for(i = 0; i < neighborhood_count; i++) begin : neighborhood_generate
            neighborhood #(max_neighborhood_m10k_size, neighborhood_m10k_address_len, i) local_neighborhood(
                .clk(clk),
                .reset(reset),
                .visitor_done(last_visitor),
                .done(local_done[i]),
                .next(next[i]),
                .sending(sending),
                .filling(pos_we),
                .max_index(neighborhood_max_index),
                .relative_visitor_valid(relative_visitor_valid[i]),
                .relative_visitor_index(converted_visitor_index),
                .visitor_x_pos(visitor_x_pos),
                .visitor_y_pos(visitor_y_pos),
                .visitor_mass(visitor_mass),

                .pos_write_addr(converted_hps_index),
                .pos_we(pos_we && hps_index_valid[i]),
                .x_pos_write_data_in(x_pos_write_data_in),
                .y_pos_write_data_in(y_pos_write_data_in),

                .extern_accel_addr(converted_accel_index),
                .extern_x_accel_output(local_x_accel_out[i]),
                .extern_y_accel_output(local_y_accel_out[i])
            );
        end
    endgenerate
endmodule