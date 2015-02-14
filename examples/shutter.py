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


signal.signal(signal.SIGTERM, signal_handler)
signal.signal(signal.SIGINT, signal_handler)

reference = 0
shutter_duration = 500
sleeptime = 1

one_time_move = []

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
			command_text = ','.join([str(v) for v in [2, shutter_duration, reference]])
			ser.write(command_text + '\n')
			reference += 1
		elif len(parsed) >= 2 and parsed[1] == 'success':
			# move has completed...wait 1s and issue another move command.
			print line.replace('\r\n', '')

			time.sleep(sleeptime)
			command_text = ','.join([str(v) for v in [2, shutter_duration, reference]])
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
