
//pipeline for the force force calculations
module gravity_force(
    input logic clk,
    //Input on cycle 1
    input logic [31:0] p1x,
    input logic [31:0] p1y,
    input logic [31:0] p2x,
    input logic [31:0] p2y,
    //Input on cycle 15
    // input logic [31:0] m1,
    //visitor mass
    input logic [31:0] m2,
    //Input on cycle 16
    //Input x and y accelerations 
    // neighborhood accel
    input logic [31:0] a1x,
    input logic [31:0] a1y,
    // visitor accel
    // input logic [31:0] a2x,
    // input logic [31:0] a2y,
    //Output on end of cycle 17
    //Output summed accelerations
    // neighborhood accel
    output logic [31:0] a1x_out,
    output logic [31:0] a1y_out
    // visitor accel
    // output logic [31:0] a2x_out,
    // output logic [31:0] a2y_out
);
    
    
    //Cycle 0
    wire [31:0] m2_sreg_out;
    shift_reg #(32, 15) m2_sreg(.clk(clk), .data_in(m2), .data_out(m2_sreg_out));
    //neagtives to make the displacement addtion a subtraction 
    wire [31:0] p2x_neg;
    wire [31:0] p2y_neg;   

    FpNegate x2_negate(.iA(p2x),.oNegative(p2x_neg));
    FpNegate y2_negate(.iA(p2y),.oNegative(p2y_neg));

    //displacement output wires
    wire [31:0] x_displacement;
    wire [31:0] y_displacement;
    //calculate each of the differences
    FpAdd x_displacement_calc( .iCLK(clk), .iA(p1x), .iB(p2x_neg), .oSum(x_displacement));
    FpAdd y_displacement_calc( .iCLK(clk), .iA(p1y), .iB(p2y_neg), .oSum(y_displacement));   


    //Cycle 2

    //register
    wire [31:0] x_displacement_squared;
    wire [31:0] y_displacement_squared;
    logic [31:0] x_displacement_squared_reg;
    logic [31:0] y_displacement_squared_reg;


    wire [31:0] x_disp_shift_reg_output;
    wire [31:0] y_disp_shift_reg_output;

    //make shift reg
    shift_reg #(32, 8) x_disp_shift_reg(.clk(clk), .data_in(x_displacement), .data_out(x_disp_shift_reg_output)); 
    shift_reg #(32, 8) y_disp_shift_reg(.clk(clk), .data_in(y_displacement), .data_out(y_disp_shift_reg_output));

    FpMul x_displacement_square_calc(.iA(x_displacement), .iB(x_displacement), .oProd(x_displacement_squared));
    FpMul y_displacement_square_calc(.iA(y_displacement), .iB(y_displacement), .oProd(y_displacement_squared));
    always @(posedge clk) begin
        x_displacement_squared_reg <= x_displacement_squared;
        y_displacement_squared_reg <= y_displacement_squared;
    end
    

    //Cycle 3
    wire [31:0] r_squared;
    logic [31:0] radius_squared_shift_reg_output;
    FpAdd r_squared_calc( .iCLK(clk), .iA(x_displacement_squared_reg), .iB(y_displacement_squared_reg), .oSum(r_squared));
    
    shift_reg #(32, 5) radius_squared_shift_reg(.clk(clk), .data_in(r_squared), .data_out(radius_squared_shift_reg_output));
    
    //Cycle 5
    
    
    //calculate inverse squre root
    wire [31:0] inv_radius;
    logic [31:0] inv_radius_reg;
    FpInvSqrt inv_sqrt_r_squared_calc(.iCLK(clk),.iA(r_squared),.oInvSqrt(inv_radius));
    always @(posedge clk) begin
        inv_radius_reg <= inv_radius;
    end


    localparam [31:0] epsilon_squared = {1'd0, 8'd100, 23'd0};   
    wire [31:0] r_squared_plus_ep_sq;
    //Add epsilon squared, Ends on Cycle 7
    FpAdd r_squared_plus_ep( .iCLK(clk), .iA(epsilon_squared), .iB(r_squared), .oSum(r_squared_plus_ep_sq));
    

    //Cycle 7
    wire [31:0] r_squared_plus_ep_inv_sqrt;
    logic [31:0] r_squared_plus_ep_inv_sqrt_reg;
    FpInvSqrt inv_sqrt_r_squared_plus_ep_calc(.iCLK(clk),.iA(r_squared_plus_ep_sq),.oInvSqrt(r_squared_plus_ep_inv_sqrt));
    always @(posedge clk) begin
        r_squared_plus_ep_inv_sqrt_reg <= r_squared_plus_ep_inv_sqrt;
    end

    //Cycle 11
    wire [31:0] x_hat;
    wire [31:0] y_hat;
    wire [31:0] x_hat_shift_reg_out;
    wire [31:0] y_hat_shift_reg_out;
    wire [31:0] radius;
    logic [31:0] radius_reg;
    logic [31:0] radius_reg_2;
    FpMul x_displacment_normalized(.iA(x_disp_shift_reg_output), .iB(inv_radius_reg), .oProd(x_hat));
    FpMul y_displacment_normalized(.iA(y_disp_shift_reg_output), .iB(inv_radius_reg), .oProd(y_hat));
    FpMul radius_mul(.iA(inv_radius_reg), .iB(radius_squared_shift_reg_output), .oProd(radius));
    always @(posedge clk) begin
        radius_reg <= radius;
        radius_reg_2 <= radius_reg;
    end
    shift_reg #(32, 4) x_hat_sreg(.clk(clk), .data_in(x_hat), .data_out(x_hat_shift_reg_out));
    shift_reg #(32, 4) y_hat_sreg(.clk(clk), .data_in(y_hat), .data_out(y_hat_shift_reg_out));


    //cycle 12
    wire [31:0] inv_r_squared_plus_ep;
    logic [31:0] inv_r_squared_plus_ep_reg;
    wire [31:0] r_inv_sqrt_r_plus_ep;
    logic [31:0] r_inv_sqrt_r_plus_ep_reg;
    FpMul inv_r_squared_plus_ep_mul(.iA(r_squared_plus_ep_inv_sqrt_reg),.iB(r_squared_plus_ep_inv_sqrt_reg),.oProd(inv_r_squared_plus_ep));
    FpMul rji_k(.iA(radius_reg_2),.iB(r_squared_plus_ep_inv_sqrt_reg),.oProd(r_inv_sqrt_r_plus_ep));
    always @(posedge clk) begin
        inv_r_squared_plus_ep_reg <= inv_r_squared_plus_ep;
        r_inv_sqrt_r_plus_ep_reg <= r_inv_sqrt_r_plus_ep;
    end

    //cycle 13
    
    wire [31:0] radius_inv_sqrt_plus_ep_cubed;
    logic [31:0] radius_inv_sqrt_plus_ep_cubed_reg;
    
    FpMul rji_k_cubed(.iA(r_inv_sqrt_r_plus_ep_reg), .iB(inv_r_squared_plus_ep_reg), .oProd(radius_inv_sqrt_plus_ep_cubed));
    always @(posedge clk) begin
        radius_inv_sqrt_plus_ep_cubed_reg <= radius_inv_sqrt_plus_ep_cubed;
    end
    

    //cycle 14
    wire [31:0] x_force_portion;
    wire [31:0] y_force_portion;
    logic [31:0] x_force_portion_reg;
    logic [31:0] y_force_portion_reg;
    FpMul x_force_portion_calc(.iA(radius_inv_sqrt_plus_ep_cubed_reg), .iB(x_hat_shift_reg_out), .oProd(x_force_portion));
    FpMul y_force_portion_calc(.iA(radius_inv_sqrt_plus_ep_cubed_reg), .iB(y_hat_shift_reg_out), .oProd(y_force_portion));
    
    always @(posedge clk) begin
        x_force_portion_reg <= x_force_portion;
        y_force_portion_reg <= y_force_portion;
    end


    //cycle 15
    // wire [31:0] ax_m1;
    // wire [31:0] ay_m1;
    wire [31:0] ax_m2;
    wire [31:0] ay_m2;
    // logic [31:0] ax_m1_reg;
    // logic [31:0] ay_m1_reg;
    logic [31:0] ax_m2_reg;
    logic [31:0] ay_m2_reg;
    logic [31:0] neg_x_force_portion;
    logic [31:0] neg_y_force_portion;
    //Muls for visitor accel - removed
    // FpMul accel_x_m1(.iA(m1), .iB(x_force_portion_reg), .oProd(ax_m1));
    // FpMul accel_y_m1(.iA(m1), .iB(y_force_portion_reg), .oProd(ay_m1));
    //point 2
    FpNegate invert_p2x(.iA(x_force_portion_reg), .oNegative(neg_x_force_portion));
    FpNegate invert_p2y(.iA(y_force_portion_reg), .oNegative(neg_y_force_portion));
    FpMul accel_x_m2(.iA(m2_sreg_out), .iB(neg_x_force_portion), .oProd(ax_m2));
    FpMul accel_y_m2(.iA(m2_sreg_out), .iB(neg_y_force_portion), .oProd(ay_m2));
    
    always @(posedge clk) begin
        // ax_m1_reg <= ax_m1;
        // ay_m1_reg <= ay_m1;
        ax_m2_reg <= ax_m2;
        ay_m2_reg <= ay_m2;
    end


    //cycle 16
    FpAdd a1x_add( .iCLK(clk), .iA(ax_m2_reg), .iB(a1x), .oSum(a1x_out));
    FpAdd a1y_add( .iCLK(clk), .iA(ay_m2_reg), .iB(a1y), .oSum(a1y_out));
    
    //Visitor accel add
    // FpAdd a2x_add( .iCLK(clk), .iA(ax_m1_reg), .iB(a2x), .oSum(a2x_out));
    // FpAdd a2y_add( .iCLK(clk), .iA(ay_m1_reg), .iB(a2y), .oSum(a2y_out));


endmodule