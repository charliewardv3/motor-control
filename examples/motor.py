import serial
import time
import math
import sys
import signal

# import pdb; pdb.set_trace()
try:
	# ser = serial.Serial('/dev/cu.usbmodem1451', 115200)
	ser = serial.Serial('/dev/cu.usbmodem1451', 115200, timeout=0)
except Exception, e:
	print e
	print repr(e)
	sys.exit(1)

def signal_handler(signal, frame):
	print 'Signal: {}'.format(signal)

	command_text = '-1,0'
	ser.write(command_text + '\n')
	
	sys.exit(0)

def scale_steps(resolution, full_step_count):
	scaled_steps = 0

	if resolution == 1:
		scaled_steps = full_step_count
	elif resolution == 2:
		scaled_steps = full_step_count * resolution  # math.pow(2, resolution)
	elif resolution == 4:
		scaled_steps = full_step_count * resolution  # math.pow(2, resolution)
	elif resolution == 8:
		scaled_steps = full_step_count * resolution  # math.pow(2, resolution)
	else:
		print 'invalid resolution'

	return scaled_steps

def scale_delay(resolution, full_step_delay):
	scaled_delay = full_step_delay
	if resolution == 1:
		scaled_delay = full_step_delay
		# scaled_delay = 1200
	elif resolution == 2:
		scaled_delay = full_step_delay / 2
		# scaled_delay = 600
	elif resolution == 4:
		scaled_delay = full_step_delay / 4
		# scaled_delay = 300
	elif resolution == 8:
		# scaled_delay = 150
		scaled_delay = full_step_delay / 8
	else:
		print 'invalid resolution'

	return scaled_delay

signal.signal(signal.SIGTERM, signal_handler)
signal.signal(signal.SIGINT, signal_handler)

# [resolution],[direction],[steps],[delay],[reference]\n

full_steps = 800
# full_step_delay = 9600 * 2
full_step_delay = 1600
# full_step_delay = 1600

direction = 0
reference = 0

resolution = 1 # full steps
# resolution = 2 # half steps
# resolution = 4 # quarter steps
# resolution = 8 # micro steps

steps = scale_steps(resolution, full_steps)
delay = scale_delay(resolution, full_step_delay)
delay = 100 * resolution
# delay = 2500 * resolution

print 'resolution:', resolution
print 'steps:', steps
print 'delay:', delay

sleeptime = 1

one_time_move = []

# one_time_move = [
# 	1,
# 	0,
# 	0,
# 	0,
# 	0
# ]

timeout = ser.getTimeout()

loopcount = 0
partial_line = ''
line = ''
while True:
	if timeout is None or timeout > 0:
		# blocking reads
		line = ser.readline()
	else:
		loopcount += 1
		# sys.stdout.write('\r' + '.' * (loopcount / 3000))
		# sys.stdout.flush()

		if ser.inWaiting() > 0:
			char = None
			while ser.inWaiting() > 0:
				char = ser.read()
				partial_line += char
				if char == '\n':
					line = partial_line
					partial_line = ''
					sys.stdout.write('\n')
					sys.stdout.flush()
					break

	if len(line.strip()) > 0:
		# print line
		parsed = line.strip().split(":")
		if line.strip() == "Ready":
			# wait for the Ready message before starting.
			print line.replace('\r\n', '')
			if one_time_move:
				command_text = ','.join([str(v) for v in one_time_move])
				ser.write(command_text + '\n')
				break
			else:			
				command_text = ','.join([str(v) for v in [1, resolution, direction, steps, delay, reference]])
				ser.write(command_text + '\n')
				reference += 1
		elif len(parsed) >= 2 and parsed[1] == 'success':
			# move has completed...wait 1s and issue another move command.
			print line.replace('\r\n', '')

			time.sleep(sleeptime)
			if direction == 0:
				direction = 1
			else:
				direction = 0
			command_text = ','.join([str(v) for v in [1, resolution, direction, steps, delay, reference]])
			print command_text
			ser.write(command_text + '\n')
			reference += 1
		else:
			# unknown message
			# print '*' * 80
			print line.replace('\r\n', '')
			# print '*' * 80

		line = ''
		loopcount = 0
