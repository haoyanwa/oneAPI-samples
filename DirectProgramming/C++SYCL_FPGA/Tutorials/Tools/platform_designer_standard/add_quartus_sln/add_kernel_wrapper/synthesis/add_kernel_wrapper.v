// add_kernel_wrapper.v

// Generated using ACDS version 23.1 993

`timescale 1 ps / 1 ps
module add_kernel_wrapper (
		input  wire        clk_clk,            //           clk.clk
		output wire [63:0] exception_add_data, // exception_add.data
		output wire        irq_add_irq,        //       irq_add.irq
		input  wire        reset_reset_n       //         reset.reset_n
	);

	wire  [31:0] master_0_master_readdata;                                          // mm_interconnect_0:master_0_master_readdata -> master_0:master_readdata
	wire         master_0_master_waitrequest;                                       // mm_interconnect_0:master_0_master_waitrequest -> master_0:master_waitrequest
	wire  [31:0] master_0_master_address;                                           // master_0:master_address -> mm_interconnect_0:master_0_master_address
	wire         master_0_master_read;                                              // master_0:master_read -> mm_interconnect_0:master_0_master_read
	wire   [3:0] master_0_master_byteenable;                                        // master_0:master_byteenable -> mm_interconnect_0:master_0_master_byteenable
	wire         master_0_master_readdatavalid;                                     // mm_interconnect_0:master_0_master_readdatavalid -> master_0:master_readdatavalid
	wire         master_0_master_write;                                             // master_0:master_write -> mm_interconnect_0:master_0_master_write
	wire  [31:0] master_0_master_writedata;                                         // master_0:master_writedata -> mm_interconnect_0:master_0_master_writedata
	wire  [63:0] mm_interconnect_0_add_report_di_0_csr_ring_root_avs_readdata;      // add_report_di_0:csr_ring_root_avs_readdata -> mm_interconnect_0:add_report_di_0_csr_ring_root_avs_readdata
	wire         mm_interconnect_0_add_report_di_0_csr_ring_root_avs_waitrequest;   // add_report_di_0:csr_ring_root_avs_waitrequest -> mm_interconnect_0:add_report_di_0_csr_ring_root_avs_waitrequest
	wire   [4:0] mm_interconnect_0_add_report_di_0_csr_ring_root_avs_address;       // mm_interconnect_0:add_report_di_0_csr_ring_root_avs_address -> add_report_di_0:csr_ring_root_avs_address
	wire         mm_interconnect_0_add_report_di_0_csr_ring_root_avs_read;          // mm_interconnect_0:add_report_di_0_csr_ring_root_avs_read -> add_report_di_0:csr_ring_root_avs_read
	wire   [7:0] mm_interconnect_0_add_report_di_0_csr_ring_root_avs_byteenable;    // mm_interconnect_0:add_report_di_0_csr_ring_root_avs_byteenable -> add_report_di_0:csr_ring_root_avs_byteenable
	wire         mm_interconnect_0_add_report_di_0_csr_ring_root_avs_readdatavalid; // add_report_di_0:csr_ring_root_avs_readdatavalid -> mm_interconnect_0:add_report_di_0_csr_ring_root_avs_readdatavalid
	wire         mm_interconnect_0_add_report_di_0_csr_ring_root_avs_write;         // mm_interconnect_0:add_report_di_0_csr_ring_root_avs_write -> add_report_di_0:csr_ring_root_avs_write
	wire  [63:0] mm_interconnect_0_add_report_di_0_csr_ring_root_avs_writedata;     // mm_interconnect_0:add_report_di_0_csr_ring_root_avs_writedata -> add_report_di_0:csr_ring_root_avs_writedata
	wire         rst_controller_reset_out_reset;                                    // rst_controller:reset_out -> add_report_di_0:resetn
	wire         rst_controller_001_reset_out_reset;                                // rst_controller_001:reset_out -> [mm_interconnect_0:add_report_di_0_resetn_reset_bridge_in_reset_reset, mm_interconnect_0:master_0_clk_reset_reset_bridge_in_reset_reset, mm_interconnect_0:master_0_master_translator_reset_reset_bridge_in_reset_reset]

	add_report_di add_report_di_0 (
		.clock                           (clk_clk),                                                           //                clock.clk
		.resetn                          (~rst_controller_reset_out_reset),                                   //               resetn.reset_n
		.device_exception_bus            (exception_add_data),                                                // device_exception_bus.data
		.kernel_irqs                     (irq_add_irq),                                                       //          kernel_irqs.irq
		.csr_ring_root_avs_read          (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_read),          //    csr_ring_root_avs.read
		.csr_ring_root_avs_readdata      (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_readdata),      //                     .readdata
		.csr_ring_root_avs_readdatavalid (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_readdatavalid), //                     .readdatavalid
		.csr_ring_root_avs_write         (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_write),         //                     .write
		.csr_ring_root_avs_writedata     (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_writedata),     //                     .writedata
		.csr_ring_root_avs_address       (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_address),       //                     .address
		.csr_ring_root_avs_byteenable    (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_byteenable),    //                     .byteenable
		.csr_ring_root_avs_waitrequest   (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_waitrequest)    //                     .waitrequest
	);

	add_kernel_wrapper_master_0 #(
		.USE_PLI     (0),
		.PLI_PORT    (50000),
		.FIFO_DEPTHS (2)
	) master_0 (
		.clk_clk              (clk_clk),                       //          clk.clk
		.clk_reset_reset      (~reset_reset_n),                //    clk_reset.reset
		.master_address       (master_0_master_address),       //       master.address
		.master_readdata      (master_0_master_readdata),      //             .readdata
		.master_read          (master_0_master_read),          //             .read
		.master_write         (master_0_master_write),         //             .write
		.master_writedata     (master_0_master_writedata),     //             .writedata
		.master_waitrequest   (master_0_master_waitrequest),   //             .waitrequest
		.master_readdatavalid (master_0_master_readdatavalid), //             .readdatavalid
		.master_byteenable    (master_0_master_byteenable),    //             .byteenable
		.master_reset_reset   ()                               // master_reset.reset
	);

	add_kernel_wrapper_mm_interconnect_0 mm_interconnect_0 (
		.clk_0_clk_clk                                                (clk_clk),                                                           //                                              clk_0_clk.clk
		.add_report_di_0_resetn_reset_bridge_in_reset_reset           (rst_controller_001_reset_out_reset),                                //           add_report_di_0_resetn_reset_bridge_in_reset.reset
		.master_0_clk_reset_reset_bridge_in_reset_reset               (rst_controller_001_reset_out_reset),                                //               master_0_clk_reset_reset_bridge_in_reset.reset
		.master_0_master_translator_reset_reset_bridge_in_reset_reset (rst_controller_001_reset_out_reset),                                // master_0_master_translator_reset_reset_bridge_in_reset.reset
		.master_0_master_address                                      (master_0_master_address),                                           //                                        master_0_master.address
		.master_0_master_waitrequest                                  (master_0_master_waitrequest),                                       //                                                       .waitrequest
		.master_0_master_byteenable                                   (master_0_master_byteenable),                                        //                                                       .byteenable
		.master_0_master_read                                         (master_0_master_read),                                              //                                                       .read
		.master_0_master_readdata                                     (master_0_master_readdata),                                          //                                                       .readdata
		.master_0_master_readdatavalid                                (master_0_master_readdatavalid),                                     //                                                       .readdatavalid
		.master_0_master_write                                        (master_0_master_write),                                             //                                                       .write
		.master_0_master_writedata                                    (master_0_master_writedata),                                         //                                                       .writedata
		.add_report_di_0_csr_ring_root_avs_address                    (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_address),       //                      add_report_di_0_csr_ring_root_avs.address
		.add_report_di_0_csr_ring_root_avs_write                      (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_write),         //                                                       .write
		.add_report_di_0_csr_ring_root_avs_read                       (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_read),          //                                                       .read
		.add_report_di_0_csr_ring_root_avs_readdata                   (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_readdata),      //                                                       .readdata
		.add_report_di_0_csr_ring_root_avs_writedata                  (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_writedata),     //                                                       .writedata
		.add_report_di_0_csr_ring_root_avs_byteenable                 (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_byteenable),    //                                                       .byteenable
		.add_report_di_0_csr_ring_root_avs_readdatavalid              (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_readdatavalid), //                                                       .readdatavalid
		.add_report_di_0_csr_ring_root_avs_waitrequest                (mm_interconnect_0_add_report_di_0_csr_ring_root_avs_waitrequest)    //                                                       .waitrequest
	);

	altera_reset_controller #(
		.NUM_RESET_INPUTS          (1),
		.OUTPUT_RESET_SYNC_EDGES   ("both"),
		.SYNC_DEPTH                (2),
		.RESET_REQUEST_PRESENT     (0),
		.RESET_REQ_WAIT_TIME       (1),
		.MIN_RST_ASSERTION_TIME    (3),
		.RESET_REQ_EARLY_DSRT_TIME (1),
		.USE_RESET_REQUEST_IN0     (0),
		.USE_RESET_REQUEST_IN1     (0),
		.USE_RESET_REQUEST_IN2     (0),
		.USE_RESET_REQUEST_IN3     (0),
		.USE_RESET_REQUEST_IN4     (0),
		.USE_RESET_REQUEST_IN5     (0),
		.USE_RESET_REQUEST_IN6     (0),
		.USE_RESET_REQUEST_IN7     (0),
		.USE_RESET_REQUEST_IN8     (0),
		.USE_RESET_REQUEST_IN9     (0),
		.USE_RESET_REQUEST_IN10    (0),
		.USE_RESET_REQUEST_IN11    (0),
		.USE_RESET_REQUEST_IN12    (0),
		.USE_RESET_REQUEST_IN13    (0),
		.USE_RESET_REQUEST_IN14    (0),
		.USE_RESET_REQUEST_IN15    (0),
		.ADAPT_RESET_REQUEST       (0)
	) rst_controller (
		.reset_in0      (~reset_reset_n),                 // reset_in0.reset
		.clk            (clk_clk),                        //       clk.clk
		.reset_out      (rst_controller_reset_out_reset), // reset_out.reset
		.reset_req      (),                               // (terminated)
		.reset_req_in0  (1'b0),                           // (terminated)
		.reset_in1      (1'b0),                           // (terminated)
		.reset_req_in1  (1'b0),                           // (terminated)
		.reset_in2      (1'b0),                           // (terminated)
		.reset_req_in2  (1'b0),                           // (terminated)
		.reset_in3      (1'b0),                           // (terminated)
		.reset_req_in3  (1'b0),                           // (terminated)
		.reset_in4      (1'b0),                           // (terminated)
		.reset_req_in4  (1'b0),                           // (terminated)
		.reset_in5      (1'b0),                           // (terminated)
		.reset_req_in5  (1'b0),                           // (terminated)
		.reset_in6      (1'b0),                           // (terminated)
		.reset_req_in6  (1'b0),                           // (terminated)
		.reset_in7      (1'b0),                           // (terminated)
		.reset_req_in7  (1'b0),                           // (terminated)
		.reset_in8      (1'b0),                           // (terminated)
		.reset_req_in8  (1'b0),                           // (terminated)
		.reset_in9      (1'b0),                           // (terminated)
		.reset_req_in9  (1'b0),                           // (terminated)
		.reset_in10     (1'b0),                           // (terminated)
		.reset_req_in10 (1'b0),                           // (terminated)
		.reset_in11     (1'b0),                           // (terminated)
		.reset_req_in11 (1'b0),                           // (terminated)
		.reset_in12     (1'b0),                           // (terminated)
		.reset_req_in12 (1'b0),                           // (terminated)
		.reset_in13     (1'b0),                           // (terminated)
		.reset_req_in13 (1'b0),                           // (terminated)
		.reset_in14     (1'b0),                           // (terminated)
		.reset_req_in14 (1'b0),                           // (terminated)
		.reset_in15     (1'b0),                           // (terminated)
		.reset_req_in15 (1'b0)                            // (terminated)
	);

	altera_reset_controller #(
		.NUM_RESET_INPUTS          (1),
		.OUTPUT_RESET_SYNC_EDGES   ("deassert"),
		.SYNC_DEPTH                (2),
		.RESET_REQUEST_PRESENT     (0),
		.RESET_REQ_WAIT_TIME       (1),
		.MIN_RST_ASSERTION_TIME    (3),
		.RESET_REQ_EARLY_DSRT_TIME (1),
		.USE_RESET_REQUEST_IN0     (0),
		.USE_RESET_REQUEST_IN1     (0),
		.USE_RESET_REQUEST_IN2     (0),
		.USE_RESET_REQUEST_IN3     (0),
		.USE_RESET_REQUEST_IN4     (0),
		.USE_RESET_REQUEST_IN5     (0),
		.USE_RESET_REQUEST_IN6     (0),
		.USE_RESET_REQUEST_IN7     (0),
		.USE_RESET_REQUEST_IN8     (0),
		.USE_RESET_REQUEST_IN9     (0),
		.USE_RESET_REQUEST_IN10    (0),
		.USE_RESET_REQUEST_IN11    (0),
		.USE_RESET_REQUEST_IN12    (0),
		.USE_RESET_REQUEST_IN13    (0),
		.USE_RESET_REQUEST_IN14    (0),
		.USE_RESET_REQUEST_IN15    (0),
		.ADAPT_RESET_REQUEST       (0)
	) rst_controller_001 (
		.reset_in0      (~reset_reset_n),                     // reset_in0.reset
		.clk            (clk_clk),                            //       clk.clk
		.reset_out      (rst_controller_001_reset_out_reset), // reset_out.reset
		.reset_req      (),                                   // (terminated)
		.reset_req_in0  (1'b0),                               // (terminated)
		.reset_in1      (1'b0),                               // (terminated)
		.reset_req_in1  (1'b0),                               // (terminated)
		.reset_in2      (1'b0),                               // (terminated)
		.reset_req_in2  (1'b0),                               // (terminated)
		.reset_in3      (1'b0),                               // (terminated)
		.reset_req_in3  (1'b0),                               // (terminated)
		.reset_in4      (1'b0),                               // (terminated)
		.reset_req_in4  (1'b0),                               // (terminated)
		.reset_in5      (1'b0),                               // (terminated)
		.reset_req_in5  (1'b0),                               // (terminated)
		.reset_in6      (1'b0),                               // (terminated)
		.reset_req_in6  (1'b0),                               // (terminated)
		.reset_in7      (1'b0),                               // (terminated)
		.reset_req_in7  (1'b0),                               // (terminated)
		.reset_in8      (1'b0),                               // (terminated)
		.reset_req_in8  (1'b0),                               // (terminated)
		.reset_in9      (1'b0),                               // (terminated)
		.reset_req_in9  (1'b0),                               // (terminated)
		.reset_in10     (1'b0),                               // (terminated)
		.reset_req_in10 (1'b0),                               // (terminated)
		.reset_in11     (1'b0),                               // (terminated)
		.reset_req_in11 (1'b0),                               // (terminated)
		.reset_in12     (1'b0),                               // (terminated)
		.reset_req_in12 (1'b0),                               // (terminated)
		.reset_in13     (1'b0),                               // (terminated)
		.reset_req_in13 (1'b0),                               // (terminated)
		.reset_in14     (1'b0),                               // (terminated)
		.reset_req_in14 (1'b0),                               // (terminated)
		.reset_in15     (1'b0),                               // (terminated)
		.reset_req_in15 (1'b0)                                // (terminated)
	);

endmodule
