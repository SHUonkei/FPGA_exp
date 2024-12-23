//***********************************************************************
// top.v
// Top level system including MIPS, memory, and I/Os
//
// 2013-07-04   Created (by matutani)
// 2013-10-07   Byte enable is added (by matutani)
// 2016-06-03   Target is changed from Spartan-3AN to Zynq-7000 (by matutani)
// 2019-08-30   100msec timer is added (by matutani)
// 2024-07-21   SPI output driver is added (by matutani)
//***********************************************************************
`timescale 1ns/1ps
module fpga_top (
	input			clk_125mhz,
	input		[3:0]	sw,
	input		[3:0]	btn,
	output 	reg	[3:0]	led,
	output		[7:0]	lcd,
	// output	reg	[7:0]	ioa,
	output reg	[3:0]	ioa_lo,
	input		[3:0]	ioa_hi,
	output		[7:0]	iob,
	output		[7:0]	ioc,
	output reg	[7:0]	iod
);
wire	[31:0]	pc, instr, readdata, readdata0, readdata1, writedata, dataadr, readdata5;
wire	[3:0]	byteen;
wire		reset;
wire		memwrite, memtoregM, swc, cs0, cs1, cs2, cs3, cs4, cs5, cs6, irq;
reg		clk_62p5mhz;
reg [7:0] mode;
wire buzz;

/* Reset when two buttons are pushed */
assign	reset	= btn[0] & btn[1];

/* 62.5MHz clock */
always @ (posedge clk_125mhz)
	if (reset)	clk_62p5mhz	<= 1;
	else		clk_62p5mhz	<= ~clk_62p5mhz;

/* CPU module (@62.5MHz) */
mips mips (clk_62p5mhz, reset, pc, instr, {7'b0000000, irq}, memwrite, 
	memtoregM, swc, byteen, dataadr, writedata, readdata, 1'b1, 1'b1);

/* Memory(cs0), Switch(cs1), LED(cs2), LCD(cs3), 7segLED(cs4), keypad(cs5),
 * buzz(cs6) and more ... */
assign	cs0	= dataadr <  32'hff00;
assign	cs1	= dataadr == 32'hff04;
assign	cs2	= dataadr == 32'hff08;
assign  cs3 = dataadr == 32'hff0c;
assign  cs4 = dataadr == 32'hff10;
assign	cs5 = dataadr == 32'hff14; 
assign	cs6 = dataadr == 32'hff18;

assign	readdata	= cs0 ? readdata0 : cs1 ? readdata1 : cs5 ? readdata5 : 0;

/* Memory module (@125MHz) */
mem mem (clk_125mhz, reset, cs0 & memwrite, pc[15:2], dataadr[15:2], instr, 
		readdata0, writedata, byteen);

/* Timer module (@62.5MHz) */
timer timer (clk_62p5mhz, reset, irq);

/* 7seg module */
reg     [6:0]   num;
seg7led seg7led (clk_62p5mhz, reset, num, iob, ioc);

/* buzz */
beep beep (clk_62p5mhz, reset, mode, buzz);

/* cs1 */
assign	readdata1	= {24'h0, btn, sw};
/* cs2 */
always @ (posedge clk_62p5mhz or posedge reset)
	if (reset)			led	<= 0;
	else if (cs2 && memwrite)	led	<= writedata[3:0];
/* cs4(7seg LED) */
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      num     <= 0;
        else if (cs4 && memwrite)       num     <= writedata[6:0];
/*SPs module (@62.5Mhz) */
spi spi (clk_62p5mhz, reset, cs3 && memwrite, writedata[9:0], lcd);
/* cs5 (key pad) */
assign readdata5 = {24'h0, ioa_hi, ioa_lo};
always @ (posedge clk_62p5mhz or posedge reset)
	if (reset) ioa_lo <= 0;
	else if (cs5 && memwrite) ioa_lo <= writedata[3:0];

/* cs6 (buzz) */
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      mode    <= 0;
        else if (cs6 && memwrite)       mode    <= writedata[7:0];
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset)                      iod     <= 0;
        else                            iod[0]  <= buzz;

endmodule

module beep (
       input clk_62p5mhz,
       input reset,
       input [7:0] mode,
       output buzz
);
reg  [31:0] count;
wire [31:0] interval;
assign interval =      (mode ==  1) ? 14931 * 2: /* C  */
                       (mode ==  2) ? 14093 * 2: /* C# */
                       (mode ==  3) ? 13302 * 2: /* D  */
                       (mode ==  4) ? 12555 * 2: /* D# */
                       (mode ==  5) ? 11850 * 2: /* E  */
                       (mode ==  6) ? 11185 * 2: /* F  */
                       (mode ==  7) ? 10558 * 2: /* F# */
                       (mode ==  8) ?  9965 * 2: /* G  */
                       (mode ==  9) ?  9406 * 2: /* G# */
                       (mode == 10) ?  8878 * 2: /* A  */
                       (mode == 11) ?  8380 * 2: /* A# */
                       (mode == 12) ?  7909 * 2: /* B  */
                       (mode == 13) ?  7465 * 2: /* C  */
		       0;
assign buzz = (mode > 0) && (count < interval / 2) ? 1 : 0;
always @ (posedge clk_62p5mhz or posedge reset)
       if (reset)
               count   <= 0;
       else if (mode > 0)
               if (count < interval)
                       count   <= count + 1;
               else
                       count   <= 0;
endmodule


/* 7segment LED */
module seg7led (
        input clk_62p5mhz,
        input reset,
        input [6:0] num,        /* 0 ... 99 */
        output [3:0] dout1,
        output [3:0] dout2
);
reg             digit;
reg     [19:0]  counter;
assign  dout1 = seg7out1(digit ? digit1(num) : digit10(num));
assign  dout2 = {digit, seg7out2(digit ? digit1(num) : digit10(num))};
always @ (posedge clk_62p5mhz or posedge reset)
        if (reset) begin
                counter <= 0;
                digit   <= 0;
        end else if (counter < 20'd625000)
                counter <= counter + 1;
        else begin
                counter <= 0;
                digit   <= ~digit;
        end
function [3:0] digit1 (input [6:0] num);
        if (num < 10)           digit1  = num;
        else if (num < 20)      digit1  = num - 10;
        else if (num < 30)      digit1  = num - 20;
        else if (num < 40)      digit1  = num - 30;
        else if (num < 50)      digit1  = num - 40;
        else if (num < 60)      digit1  = num - 50;
        else if (num < 70)      digit1  = num - 60;
        else if (num < 80)      digit1  = num - 70;
        else if (num < 90)      digit1  = num - 80;
        else                    digit1  = num - 90;
endfunction
function [3:0] digit10 (input [6:0] num);
        if (num < 10)           digit10 = 0;
        else if (num < 20)      digit10 = 1;
        else if (num < 30)      digit10 = 2;
        else if (num < 40)      digit10 = 3;
        else if (num < 50)      digit10 = 4;
        else if (num < 60)      digit10 = 5;
        else if (num < 70)      digit10 = 6;
        else if (num < 80)      digit10 = 7;
        else if (num < 90)      digit10 = 8;
        else                    digit10 = 9;
endfunction
function [3:0] seg7out1 (input [3:0] din);
        case (din)
        4'd0: seg7out1 = 4'b1111;
        4'd1: seg7out1 = 4'b0000;
        4'd2: seg7out1 = 4'b1011;
        4'd3: seg7out1 = 4'b1001;
        4'd4: seg7out1 = 4'b0100;
        4'd5: seg7out1 = 4'b1101;
        4'd6: seg7out1 = 4'b1111;
        4'd7: seg7out1 = 4'b1100;
        4'd8: seg7out1 = 4'b1111;
        4'd9: seg7out1 = 4'b1100;
        default: seg7out1 = 4'b0000;
        endcase
endfunction
function [2:0] seg7out2 (input [3:0] din);
        case (din)
        4'd0: seg7out2 = 3'b011;
        4'd1: seg7out2 = 3'b011;
        4'd2: seg7out2 = 3'b101;
        4'd3: seg7out2 = 3'b111;
        4'd4: seg7out2 = 3'b111;
        4'd5: seg7out2 = 3'b110;
        4'd6: seg7out2 = 3'b110;
        4'd7: seg7out2 = 3'b011;
        4'd8: seg7out2 = 3'b111;
        4'd9: seg7out2 = 3'b111;
        default: seg7out2 = 3'b000;
        endcase
endfunction
endmodule


//***********************************************************************
// 100msec timer for 62.5MHz clock
//
// 2019-08-30 Created (by matutani)
//***********************************************************************
module timer (
	input			clk, reset,
	output			irq
);
reg	[22:0]	counter;

assign	irq = (counter == 23'd6250000);

always @ (posedge clk or posedge reset)
	if (reset) 			counter	<= 0;
	else if (counter < 23'd6250000)	counter	<= counter + 1;
	else 				counter	<= 0;
endmodule

//***********************************************************************
// Memory (32bit x 16384word) with synchronous read ports for BlockRAM
//
// 2013-07-04   Created (by matutani)
// 2013-10-07   Byte enable is added (by matutani)
// 2016-06-03   Memory size is changed from 8192 to 16384 words (by matutani)
//***********************************************************************
module mem (
	input			clk, reset, memwrite,
	input		[13:0]	instradr, dataadr,
	output	reg	[31:0]	instr,
	output	reg	[31:0]	readdata,
	input		[31:0]	writedata,
	input		[3:0]	byteen
);
reg	[31:0]	RAM [0:16383];	/* Memory size is 16384 words */
wire	[7:0]	byte0, byte1, byte2, byte3;

assign	byte0	= byteen[0] ? writedata[ 7: 0] : readdata[ 7: 0];
assign	byte1	= byteen[1] ? writedata[15: 8] : readdata[15: 8];
assign	byte2	= byteen[2] ? writedata[23:16] : readdata[23:16];
assign	byte3	= byteen[3] ? writedata[31:24] : readdata[31:24];

always @ (posedge clk) begin
	if (memwrite)
		RAM[dataadr]	<= {byte3, byte2, byte1, byte0};
	instr	<= RAM[instradr];
	readdata<= RAM[dataadr];
end

/* Specify your program image file (e.g., program.dat) */
initial $readmemh("program.dat", RAM, 0, 16383);
endmodule

//***********************************************************************
// SPI 8-bit output driver for 62.5MHz clock
//
// 2024-07-21   Created (by matutani)
//***********************************************************************
`define	SPI_WAIT	2'b00
`define	SPI_START	2'b01
`define	SPI_TRANS	2'b10
`define	SPI_STOP	2'b11
`define	SPI_DATA	1'b1
`define	SPI_CMD		1'b0
`define	Enable_		1'b0
`define	Disable_	1'b1
`define	SPI_FREQDIV	25	/* 62.5MHz / 2 / 25 = 1.25MHz */

module spi (
	input clk, input reset, input start, input [9:0] din, output [7:0] dout
);
reg	[1:0]	state;
reg	[7:0]	d_reg;
reg	[2:0]	cnt;	/* 8 */
reg	[4:0]	cnt2;	/* 25 */
reg	cs_, dc_, res_, sdo, sck, pmoden, vccen;

assign	dout = {pmoden, vccen, res_, dc_, sck, 1'b0, sdo, cs_};
always @(posedge clk or posedge reset)
	if (reset) begin
		state	<= `SPI_WAIT;
		d_reg	<= 0;
		cnt	<= 0;
		cnt2	<= 0;
		cs_	<= `Disable_;
		dc_	<= 0;
		res_	<= `Enable_;
		sdo	<= 0;
		sck	<= 0;
		pmoden	<= 0; /* Display power OFF */
		vccen	<= 0; /* Display power OFF */
	end else if (state == `SPI_WAIT) begin
		res_	<= `Disable_;
		sck	<= 1;
		if (start && din[9]) begin
			pmoden	<= 1; /* Display power ON */
			vccen	<= 1; /* Display power ON */
		end else if (start) begin
			state	<= `SPI_START;
			d_reg	<= din[7:0];
			cnt	<= 8;
			cs_	<= `Enable_;
			dc_	<= din[8];
			cnt2	<= 0;
		end
	end else if (state == `SPI_START)
		if (cnt2 == `SPI_FREQDIV - 1) begin
			state	<= `SPI_TRANS;
			cnt2	<= 0;
		end else
			cnt2	<= cnt2 + 1;
	else if (state == `SPI_TRANS)
		if (sck)
			if (cnt2 == `SPI_FREQDIV - 1) begin
				sck	<= 0;
				sdo	<= d_reg[7];
				cnt	<= cnt - 1;
				cnt2	<= 0;
			end else
				cnt2	<= cnt2 + 1;
		else
			if (cnt2 == `SPI_FREQDIV - 1) begin
				sck	<= 1;
				d_reg	<= {d_reg[6:0], 1'b0};
				cnt2	<= 0;
				if (cnt == 0)
					state	<= `SPI_STOP;
			end else
				cnt2	<= cnt2 + 1;
	else if (state == `SPI_STOP)
		if (cnt2 == `SPI_FREQDIV - 1) begin
			state	<= `SPI_WAIT;
			cs_	<= `Disable_;
			cnt2	<= 0;
		end else
			cnt2	<= cnt2 + 1;
endmodule
