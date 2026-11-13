function App() {
	const W = 1500,
		H = 990;
	const C = {
		wire: '#46525f',
		sig: '#2e86e0',
		pwr: '#e5484d',
		gnd: '#30a46c',
		box: '#1c1e21',
		link: '#8e5cd0',
		pin: '#edf1f5',
		pinOn: '#5f6b78'
	};
	const F = {
		batt: '#e0483f',
		battEdge: '#a83228',
		urc: '#c62828',
		urcEdge: '#8e1b1b',
		urcBlock: '#9e2420',
		urcBlockEdge: '#f0b4ae',
		host: '#3d4650',
		hostEdge: '#5a6470',
		motor: '#d3d8dc',
		motorEdge: '#8f979e',
		lidar: '#23272c',
		lidarEdge: '#4a5058',
		legend: '#ffffff'
	};
	const netColor = { sig: C.sig, pwr: C.pwr, gnd: C.gnd };
	const wireColor = { sig: C.wire, pwr: C.pwr, gnd: C.gnd };

	const Txt = ({
		x,
		y,
		size = 12,
		color = C.box,
		bold = false,
		anchor,
		mono = true,
		inBox,
		children
	}) => (
		<text
			x={x}
			y={y}
			fontSize={size}
			fill={color}
			textAnchor={anchor}
			fontWeight={bold ? 'bold' : 'normal'}
			fontFamily={mono ? 'Consolas, Menlo, monospace' : 'Inter, Arial, sans-serif'}
			xmlSpace="preserve"
			data-in={inBox}
		>
			{children}
		</text>
	);
	const Box = ({ id, x, y, w, h, fill, edge, dashed }) => (
		<rect
			data-box={id}
			x={x}
			y={y}
			width={w}
			height={h}
			fill={fill}
			stroke={edge}
			strokeWidth={dashed ? 1.4 : 1.8}
			strokeDasharray={dashed ? '6 4' : undefined}
			rx={7}
		/>
	);
	const Zone = ({ x, y, dark, title, sub, inBox }) => (
		<g>
			<Txt
				x={x}
				y={y}
				size={14}
				color={dark ? '#eef1f4' : C.box}
				bold
				anchor="middle"
				mono={false}
				inBox={inBox}
			>
				{title}
			</Txt>
			{sub && (
				<Txt
					x={x}
					y={y + 16}
					size={11}
					color={dark ? '#cdd7e0' : '#5b6570'}
					anchor="middle"
					mono={false}
					inBox={inBox}
				>
					{sub}
				</Txt>
			)}
		</g>
	);
	const Wire = (x1, y1, x2, y2, color = C.wire, opts = {}) => (
		<line
			x1={x1}
			y1={y1}
			x2={x2}
			y2={y2}
			stroke={color}
			strokeWidth={opts.w || 1.6}
			strokeLinecap="round"
			strokeDasharray={opts.dash}
			data-skip={opts.skip ? '1' : undefined}
		/>
	);
	const Dot = ({ x, y }) => <circle cx={x} cy={y} r={3} fill={C.wire} />;
	const Gnd = ({ x, y }) => (
		<g stroke={C.gnd} strokeWidth={2}>
			<line x1={x} y1={y} x2={x} y2={y + 8} strokeWidth={1.6} />
			<line x1={x - 8} y1={y + 8} x2={x + 8} y2={y + 8} />
			<line x1={x - 5} y1={y + 12} x2={x + 5} y2={y + 12} />
			<line x1={x - 2} y1={y + 16} x2={x + 2} y2={y + 16} />
		</g>
	);
	const NoConnect = ({ x, y }) => (
		<g stroke={C.pinOn} strokeWidth={1.6}>
			<line x1={x - 4} y1={y - 4} x2={x + 4} y2={y + 4} />
			<line x1={x - 4} y1={y + 4} x2={x + 4} y2={y - 4} />
		</g>
	);

	const URC = { x: 460, y: 90, w: 540, h: 880 };
	const urcRight = URC.x + URC.w;
	const devX = 1230,
		devW = 250;
	const labelX = (urcRight + devX) / 2;

	const motorWires = [
		'Red · M+',
		'Black · VCC',
		'Yellow · A',
		'Green · B',
		'Blue · GND',
		'White · M−'
	];
	const motors = [
		{
			ref: 'J10',
			n: 1,
			net: 'FL',
			name: 'MFL · front left',
			joint: 'front_left_coupling_joint',
			cap: 140
		},
		{
			ref: 'J11',
			n: 2,
			net: 'FR',
			name: 'MFR · front right',
			joint: 'front_right_coupling_joint',
			cap: 300
		},
		{
			ref: 'J12',
			n: 3,
			net: 'RL',
			name: 'MRL · rear left',
			joint: 'rear_left_coupling_joint',
			cap: 460
		},
		{
			ref: 'J13',
			n: 4,
			net: 'RR',
			name: 'MRR · rear right',
			joint: 'rear_right_coupling_joint',
			cap: 620
		}
	];
	const motorPins = (p) => [
		['M1', `${p}_M+`, 'sig'],
		['5V', '+5V', 'pwr'],
		['ENC1', `${p}_ENC_A`, 'sig'],
		['ENC2', `${p}_ENC_B`, 'sig'],
		['GND', 'GND', 'gnd'],
		['M2', `${p}_M−`, 'sig']
	];
	const lidarCap = 780;
	const lidarPins = [
		['5V', '+5V', 'pwr', 'VCC_5'],
		['5V', '+5V', 'pwr', '5V_MOTO'],
		['U1TX/14', 'LIDAR_TX', 'sig', 'TX'],
		['IO7', null, null, null],
		['U1RX/15', 'LIDAR_RX', 'sig', 'RX'],
		['IO10', 'LIDAR_PWM', 'sig', 'CTRL_MOTO'],
		['GND', 'GND', 'gnd', 'GND'],
		['GND', 'GND', 'gnd', 'GND_MOTO']
	];
	const pinY = (cap, i) => cap + 20 + i * 20;

	const Run = ({ y, urcPin, net, kind, devPin, devId, devDark }) => (
		<g>
			<Txt x={urcRight - 6} y={y + 4} size={10} color={C.pin} anchor="end" inBox="urc">
				{urcPin}
			</Txt>
			{net ? (
				<g>
					{Wire(urcRight, y, devX, y, wireColor[kind])}
					<Txt x={labelX} y={y - 4} size={11} color={netColor[kind]} bold anchor="middle">
						{net}
					</Txt>
					<Txt
						x={devX + 6}
						y={y + 4}
						size={10}
						color={devDark ? C.pin : C.pinOn}
						anchor="start"
						inBox={devId}
					>
						{devPin}
					</Txt>
				</g>
			) : (
				<g>
					{Wire(urcRight, y, urcRight + 16, y)}
					<NoConnect x={urcRight + 20} y={y} />
					<Txt x={urcRight + 30} y={y + 4} size={10} color={C.pinOn}>
						NC
					</Txt>
				</g>
			)}
		</g>
	);

	const Block = ({ id, y, h, title, lines }) => (
		<g>
			<Box id={id} x={580} y={y} w={300} h={h} fill={F.urcBlock} edge={F.urcBlockEdge} dashed />
			<Txt x={596} y={y + 20} size={12} color="#eef1f4" bold mono={false} inBox={id}>
				{title}
			</Txt>
			{lines.map((l, i) => (
				<Txt key={l} x={596} y={y + 38 + i * 16} size={10} color="#f4dcda" inBox={id}>
					{l}
				</Txt>
			))}
		</g>
	);

	const hostLines = [
		'zenohd · tcp/0.0.0.0:7447',
		'controller_manager @ 100 Hz',
		'  zenbedded::ZenbeddedHardware',
		'  mecanum_drive_controller',
		'  joint_state_broadcaster',
		'rmw_zenoh_cpp · Nav2 · SLAM'
	];

	const notes = [
		'Motor terminals follow the N20 cable order 1:1 (red … white).',
		'Encoders run on +5V. URC diodes D7–D14 shift them to 3.3 V.',
		'J14 names are from the LiDAR side: U1TX/14 carries LiDAR TX',
		'into ESP32 GPIO14 (UART1 RX). U1RX/15 is ESP32 TX.',
		'RPLidar core: ≤50 mV ripple, up to 600 mA at start. It shares',
		'the URC +5V buck (3.5 A) with the encoders only.',
		'URC has no fuse or switch on VIN, so F1 and S1 are required.',
		'* /scan over zenoh-pico is new firmware work (plan phase 4).'
	];

	return (
		<svg
			xmlns="http://www.w3.org/2000/svg"
			viewBox={`0 0 ${W} ${H}`}
			width="100%"
			style={{ display: 'block', background: '#fff' }}
		>
			<style>{`html, body { margin: 0; background: #fff; }`}</style>

			<Box id="host" x={40} y={100} w={320} h={230} fill={F.host} edge={F.hostEdge} />
			<Zone
				x={200}
				y={122}
				dark
				title="HOST · laptop"
				sub="off-robot · Ubuntu + ROS 2"
				inBox="host"
			/>
			{hostLines.map((l, i) => (
				<Txt key={l} x={56} y={184 + i * 20} size={10} color="#dfe5ea" inBox="host">
					{l}
				</Txt>
			))}
			<Txt x={354} y={154} size={10} color={C.pin} anchor="end" inBox="host">
				Wi-Fi
			</Txt>
			<Txt x={354} y={204} size={10} color={C.pin} anchor="end" inBox="host">
				USB
			</Txt>
			{Wire(360, 150, URC.x, 150, C.link, { dash: '6 5', w: 2 })}
			<Txt x={410} y={144} size={10} color={C.link} bold anchor="middle">
				zenoh/Wi-Fi
			</Txt>
			{Wire(360, 200, URC.x, 200, C.link, { dash: '6 5', w: 2 })}
			<Txt x={410} y={194} size={10} color={C.link} bold anchor="middle">
				flash/logs
			</Txt>

			<Box id="bt1" x={40} y={440} w={170} h={105} fill={F.batt} edge={F.battEdge} />
			<Zone x={125} y={462} dark title="BT1 · 3S LiPo" sub="11.1 V · 2200 mAh" inBox="bt1" />
			<Txt x={204} y={504} size={10} color={C.pin} anchor="end" inBox="bt1">
				B+
			</Txt>
			<Txt x={204} y={529} size={10} color={C.pin} anchor="end" inBox="bt1">
				B−
			</Txt>
			{Wire(210, 500, 245, 500, C.pwr)}
			<rect
				data-box="f1"
				x={245}
				y={494}
				width={40}
				height={12}
				fill="#fff"
				stroke={C.pwr}
				strokeWidth={1.6}
				rx={2}
			/>
			<Txt x={265} y={486} size={10} color={C.pinOn} anchor="middle">
				F1 5 A
			</Txt>
			{Wire(285, 500, 322, 500, C.pwr)}
			<circle cx={325} cy={500} r={2.5} fill={C.wire} />
			<circle cx={357} cy={500} r={2.5} fill={C.wire} />
			{Wire(325, 500, 352, 488)}
			<Txt x={341} y={482} size={10} color={C.pinOn} anchor="middle">
				S1
			</Txt>
			{Wire(360, 500, URC.x, 500, C.pwr)}
			<Txt x={410} y={496} size={11} color={C.pwr} bold anchor="middle">
				VIN
			</Txt>
			{Wire(210, 525, URC.x, 525, C.gnd)}
			<Dot x={300} y={525} />
			<Gnd x={300} y={525} />
			<Txt x={410} y={521} size={11} color={C.gnd} bold anchor="middle">
				GND
			</Txt>

			<Box id="urc" x={URC.x} y={URC.y} w={URC.w} h={URC.h} fill={F.urc} edge={F.urcEdge} />
			<Zone
				x={730}
				y={112}
				dark
				title="URC · Universal Robot Controller"
				sub="rev 1.0 · ESP32-S3 + STM32G431"
				inBox="urc"
			/>
			<Txt x={466} y={154} size={10} color={C.pin} inBox="urc">
				Wi-Fi ANT
			</Txt>
			<Txt x={466} y={204} size={10} color={C.pin} inBox="urc">
				J2 USB-UART
			</Txt>
			<Txt x={466} y={254} size={10} color={C.pin} inBox="urc">
				J1 USB-CDC
			</Txt>
			{Wire(URC.x, 250, URC.x - 20, 250)}
			<Txt x={URC.x - 25} y={254} size={11} color={C.pinOn} bold anchor="end">
				spare
			</Txt>
			<Txt x={466} y={504} size={10} color={C.pin} inBox="urc">
				J3 VIN +
			</Txt>
			<Txt x={466} y={529} size={10} color={C.pin} inBox="urc">
				J3 VIN −
			</Txt>

			<Block
				id="esp"
				y={150}
				h={150}
				title="U4 · ESP32-S3-WROOM-1-N16R8"
				lines={[
					'Zephyr 4.4 · board urc_esp32s3',
					'zenbedded_rcl client · tier 2',
					'urc motor driver (SPI controller)',
					'RPLidar UART1 driver*',
					'Wi-Fi STA → zenohd (client mode)'
				]}
			/>
			{Wire(730, 300, 730, 380, C.pin, { skip: true })}
			<Txt x={722} y={345} size={10} color={C.pin} anchor="end" inBox="urc">
				SPI1 · 10 MHz · 20 B @ 100 Hz
			</Txt>
			<Block
				id="stm"
				y={380}
				h={110}
				title="U11 · STM32G431KBU6"
				lines={[
					'Zephyr 4.4 · board urc_stm32',
					'4× PWM 25 kHz + quadrature count',
					'500 ms comms watchdog → coast'
				]}
			/>
			{Wire(730, 490, 730, 530, C.pin, { skip: true })}
			<Txt x={722} y={514} size={10} color={C.pin} anchor="end" inBox="urc">
				IN1–IN8 · nSLEEP
			</Txt>
			<Block
				id="drv"
				y={530}
				h={80}
				title="U12–U15 · 4× DRV8256P"
				lines={['VM = VIN · duty 0 = brake']}
			/>
			<Block
				id="pwr"
				y={650}
				h={80}
				title="U5 LMR51635 · U6 LDO"
				lines={['VIN → +5V (3.5 A) → 3V3']}
			/>
			<Txt x={596} y={800} size={10} color="#f4dcda" inBox="urc">
				Unused in this build:
			</Txt>
			<Txt x={596} y={816} size={10} color="#f4dcda" inBox="urc">
				J6 VSERVO · J7 servo bus · J8 CAN FD
			</Txt>
			<Txt x={596} y={832} size={10} color="#f4dcda" inBox="urc">
				J15 Qwiic · J16/J17 GPIO headers
			</Txt>

			{motors.map((m) => {
				const top = m.cap + 6;
				const h = 128;
				const cy = top + h / 2;
				const id = `m${m.n}`;
				return (
					<g key={m.ref}>
						<Txt
							x={urcRight - 6}
							y={m.cap + 4}
							size={10}
							color="#eef1f4"
							bold
							anchor="end"
							inBox="urc"
						>
							{`${m.ref} · MOTOR ${m.n}`}
						</Txt>
						<Box id={id} x={devX} y={top} w={devW} h={h} fill={F.motor} edge={F.motorEdge} />
						<Txt x={1395} y={cy - 14} size={12} bold anchor="middle" mono={false} inBox={id}>
							{m.name}
						</Txt>
						<Txt
							x={1395}
							y={cy + 2}
							size={10}
							color={C.pinOn}
							anchor="middle"
							mono={false}
							inBox={id}
						>
							N20 12 V · 300 RPM · encoder
						</Txt>
						<Txt x={1395} y={cy + 18} size={9} color={C.pinOn} anchor="middle" inBox={id}>
							{m.joint}
						</Txt>
						{motorPins(m.net).map(([pin, net, kind], i) => (
							<Run
								key={pin + i}
								y={pinY(m.cap, i)}
								urcPin={pin}
								net={net}
								kind={kind}
								devPin={motorWires[i]}
								devId={id}
							/>
						))}
					</g>
				);
			})}

			<Txt
				x={urcRight - 6}
				y={lidarCap + 4}
				size={10}
				color="#eef1f4"
				bold
				anchor="end"
				inBox="urc"
			>
				J14 · LIDAR
			</Txt>
			<Box
				id="lidar"
				x={devX}
				y={lidarCap + 6}
				w={devW}
				h={168}
				fill={F.lidar}
				edge={F.lidarEdge}
			/>
			<Txt
				x={1405}
				y={lidarCap + 70}
				size={12}
				color="#eef1f4"
				bold
				anchor="middle"
				mono={false}
				inBox="lidar"
			>
				LIDAR1 · RPLidar A1M8
			</Txt>
			<Txt
				x={1405}
				y={lidarCap + 86}
				size={10}
				color="#cdd7e0"
				anchor="middle"
				mono={false}
				inBox="lidar"
			>
				3.3 V UART · 115200 8N1
			</Txt>
			<Txt
				x={1405}
				y={lidarCap + 102}
				size={10}
				color="#cdd7e0"
				anchor="middle"
				mono={false}
				inBox="lidar"
			>
				dev-kit 7-pin cable
			</Txt>
			{lidarPins.map(([pin, net, kind, devPin], i) => (
				<Run
					key={pin + i}
					y={pinY(lidarCap, i)}
					urcPin={pin}
					net={net}
					kind={kind}
					devPin={devPin}
					devId="lidar"
					devDark
				/>
			))}

			<Box id="legend" x={40} y={600} w={340} h={130} fill={F.legend} edge="#dde3ea" />
			<Txt x={210} y={622} size={14} bold anchor="middle" mono={false} inBox="legend">
				LEGEND
			</Txt>
			{Wire(60, 642, 100, 642, C.pwr, { w: 2.5 })}
			<Txt x={110} y={646} size={11} mono={false} inBox="legend">
				power (VIN, +5V)
			</Txt>
			{Wire(60, 664, 100, 664, C.sig, { w: 2.5 })}
			<Txt x={110} y={668} size={11} mono={false} inBox="legend">
				signal net
			</Txt>
			{Wire(60, 686, 100, 686, C.link, { w: 2.5, dash: '6 5' })}
			<Txt x={110} y={690} size={11} mono={false} inBox="legend">
				wireless or temporary link
			</Txt>
			<Gnd x={80} y={700} />
			<Txt x={110} y={714} size={11} mono={false} inBox="legend">
				common ground (green wires too)
			</Txt>

			{notes.map((n, i) => (
				<Txt key={n} x={40} y={770 + i * 18} size={11} color="#5b6570" mono={false}>
					{n}
				</Txt>
			))}
		</svg>
	);
}
