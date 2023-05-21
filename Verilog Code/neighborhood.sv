module neighborhood #(parameter max_m10k_size = 4096, m10k_address_len = 12, town_index = 0) (
    input logic clk,
    input logic reset,
    input logic visitor_done,  // sent after the last visitor is removed from visitor registers 

    // done goes to the city
    output logic done,  // sent after the last visitor - from the visitor center
    output logic next, // when calc resets - need next visitor - to the visitor center

    // enable state for sending to HPS - visitor handles sending done to HPS, after HPS replies this is high
    input logic sending, 

    // enable state for filling from the HPS
    input logic filling, 

    //number of objects, calculated from pio port - total/number of neighborhoods
    input logic [m10k_address_len-1:0] max_index,

    // sending done when at max_visitor_index, if the relative visitor is even in range
    input logic relative_visitor_valid,
    input logic [m10k_address_len-1:0] relative_visitor_index, 

    // visitor information
    input logic [31:0] visitor_x_pos,
    input logic [31:0] visitor_y_pos,
    input logic [31:0] visitor_mass,
    

    //Data in from HPS
    //Shared write address and write enable
    input logic [m10k_address_len-1:0] pos_write_addr,
    // write enable
    input logic pos_we, 

    // access accelerator to send to HPS
    input logic [m10k_address_len-1:0] extern_accel_addr,
    
    // from the HPS
    input logic [31:0] x_pos_write_data_in,
    input logic [31:0] y_pos_write_data_in,

    // send to HPS at the end
    output logic [31:0] extern_x_accel_output,
    output logic [31:0] extern_y_accel_output

);   

    // states
    logic [2:0] state;
    localparam reset_state = 3'd0;
    localparam fill_state = 3'd1;
    localparam calc_state = 3'd2;
    localparam send_state = 3'd3;
    localparam wait_state = 3'd4;
    
    // //mass control
    logic [31:0] mass_input;

    //sum output from solver
    logic [31:0] x_accel_sum;
    logic [31:0] y_accel_sum;

    //Position m10k internal singals 
    logic [m10k_address_len-1:0] pos_read_addr;
    logic [31:0] x_pos_read_data;
    logic [31:0] y_pos_read_data;

    //Singals for internal accleration addressing
    logic [m10k_address_len-1:0] accel_read_addr;
    logic [m10k_address_len-1:0] accel_write_addr;
    logic [m10k_address_len-1:0] accel_write_addr_reg;
    
    // data read out of acceleration read
    logic [31:0] x_accel_read_data;
    logic [31:0] y_accel_read_data;

    //Local result or 0 - solver or during fill from HPS is 0
    logic [31:0] x_accel_data_in;
    logic [31:0] y_accel_data_in;

    //Signal that enables writing to acceleration m10k so we don't overwrite data
    logic calc_writeback_ready;
    
    //signal to tell when we zero the mass
    logic zero_mass;
    logic zero_mass_1;
    logic zero_mass_2;

    //Accel write enable
    logic accel_we;

    // state machine for assigning
    assign x_accel_data_in = (state != calc_state)? '0 : x_accel_sum;
    assign y_accel_data_in = (state != calc_state)? '0 : y_accel_sum;
    assign accel_write_addr = (state == calc_state)? accel_write_addr_reg : extern_accel_addr; 
    
    //Only write back when calculating or filling, no write when sending or waiting or otherwise
    assign accel_we = ((state == calc_state && calc_writeback_ready) || state == fill_state) ? 1'b1 : 1'b0;
    
    //When do we want to increment visitor index
    assign next = (pos_read_addr == max_index) && (state == calc_state);

    //Detect when visitor and current address are the same
    assign zero_mass = (relative_visitor_valid && relative_visitor_index == pos_read_addr);
    //Delay 2 cycles to match when data is read out of m10k
    always @(posedge clk) begin
        zero_mass_1 <= zero_mass;
        zero_mass_2 <= zero_mass_1;
    end
    //mass to send to solver
    assign mass_input = (zero_mass_2) ? 32'd0 : visitor_mass;

    
    //State transitions
    always @(posedge clk) begin
      if(reset)
        state <= reset_state;
      else begin
        case (state)
            reset_state: begin
                if(filling)
                    state <= fill_state;
                else
                    state <= wait_state;
            end
            fill_state: begin
                if(!filling)
                    state <= calc_state;
            end
            calc_state: begin
                if(done)
                    state <= wait_state;
            end
            send_state: begin
                if(!sending)
                    state <= wait_state;
            end
          
          // wait state
            default: begin
                if(filling) begin
                    state <= fill_state;
                end
                else if(sending) begin
                    state <= send_state;
                end
                else begin
                    state <= wait_state;
                end
            end
        endcase
      end
    end
    
    //State behavior
    always @(posedge clk) begin
        // if(reset) begin
        //   // do nothing for now
        // end
        
        // else:
        case (state)
            reset_state: begin
                // do nothing
                done <= 'b0;
            end
            
            fill_state: begin
                done <= 'b0;
                if(!filling) begin

                    // filling is now done
                    // setup address registers for the calc_state
                    pos_read_addr <= 'd0;

                    // offset to make sure that once it comes out
                    // of pipelined solver, is at correct address
                    accel_read_addr <= -'d16; 
                    accel_write_addr_reg <= -'d20;  // highly sus
                    calc_writeback_ready <= 0;
                end
            end
            
            calc_state: begin

                // Increment by 1 every cycle until max index, then reset - pos
                if(pos_read_addr == max_index) 
                    pos_read_addr <= 'd0; 
                else 
                    pos_read_addr <= pos_read_addr + 1;

                // Increment by 1 every cycle until max index, then reset - accel read
                if(accel_read_addr == max_index) 
                    accel_read_addr <= 'd0; 
                else
                    accel_read_addr <= accel_read_addr + 1; 

                // Increment by 1 every cycle until max index, then reset - accel write
                if(accel_write_addr_reg == max_index) 
                    accel_write_addr_reg <= 'd0;
                else
                    accel_write_addr_reg <= accel_write_addr_reg + 1;
                
                // this is to make sure when reading the value back from the solver it gets set
                // to the correct location
                if(accel_write_addr_reg == '1) begin
                    calc_writeback_ready <= 1;
                end
                if((visitor_done && accel_write_addr_reg >= max_index))
                    done <= 'b1;
            end
            
            send_state: begin
                // we just yield to external control
                //Stop sending that we are done
                done <= 'b0;
            end
            default: begin
              // nothing
            end
        endcase
    end

    //Now useless but don't want to remove
    //Used convert from 27 bit to 32 bit signals
    logic [31:0] full_x_pos_read_data;
    logic [31:0] full_y_pos_read_data;
    logic [31:0] full_extern_x_accel_output;
    logic [31:0] full_x_accel_read_data;
    logic [31:0] full_extern_y_accel_output;
    logic [31:0] full_y_accel_read_data;
    assign extern_y_accel_output = full_extern_y_accel_output;
    assign extern_x_accel_output = full_extern_x_accel_output;
    assign y_accel_read_data = full_y_accel_read_data;
    assign x_accel_read_data = full_x_accel_read_data;
    assign y_pos_read_data = full_y_pos_read_data;
    assign x_pos_read_data = full_x_pos_read_data;

    m10k #(max_m10k_size, m10k_address_len) neighborhood_x_pos( 
        .clock(clk),

        // reading port for use with the gravity solver
        .address_a(pos_read_addr),
        .data_a(),
        .q_a(full_x_pos_read_data),
        .wren_a('0),

        // writing port for the hps to write to
        .address_b(pos_write_addr),
        .data_b(x_pos_write_data_in),
        .q_b(),
        .wren_b(pos_we)
    );
    m10k #(max_m10k_size, m10k_address_len) neighborhood_y_pos( 
        .clock(clk),

        // reading port for use with the gravity solver
        .address_a(pos_read_addr),
        .data_a(),
        .q_a(full_y_pos_read_data),
        .wren_a('0),

        // writing port for the hps to write to
        .address_b(pos_write_addr),
        .data_b(y_pos_write_data_in),
        .q_b(),
        .wren_b(pos_we)
    );
    m10k #(max_m10k_size, m10k_address_len) neighborhood_x_accel( 
        .clock(clk),
        
        // read data into the gravity solver
        .address_a(accel_read_addr),
        .data_a(),
        .q_a(full_x_accel_read_data),
        .wren_a('0),

        // shared write to zero m10k, read to HPS, write from solver
        .address_b(accel_write_addr),
        .data_b(x_accel_data_in),
        .q_b(full_extern_x_accel_output),
        .wren_b(accel_we)
    );

    m10k #(max_m10k_size, m10k_address_len) neighborhood_y_accel( 
        .clock(clk),
        
        // read data into the gravity solver
        .address_a(accel_read_addr),
        .data_a(),
        .q_a(full_y_accel_read_data),
        .wren_a('0),
        
        // shared write to zero m10k, read to HPS, write from solver
        .address_b(accel_write_addr),
        .data_b(y_accel_data_in),
        .q_b(full_extern_y_accel_output),
        .wren_b(accel_we)
    );

    // accelerator
    gravity_force solver( 
        .clk(clk), 
        .p1x(x_pos_read_data), 
        .p1y(y_pos_read_data), 
        .p2x(visitor_x_pos), 
        .p2y(visitor_y_pos), 
        .m2(mass_input), 
        .a1x(x_accel_read_data), 
        .a1y(y_accel_read_data), 
        .a1x_out(x_accel_sum), 
        .a1y_out(y_accel_sum) 
    );

endmodule
