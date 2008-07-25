



FileStream classMethod!
primOpen: fileName mode: anInteger
	<primitive: 'FileStream_open'>
	self primitiveFailed!

FileStream classMethod!
open: fileName mode: aSymbol
	| fd mode |
	
	aSymbol == #read
		ifTrue: [mode := 0]
		ifFalse:
			[aSymbol == #write
				ifTrue: [mode := 1]
				ifFalse: [self error: 'mode not supported']].
	
	fd := self primOpen: fileName mode: mode.
	^ self basicNew on: fd!

FileStream method!
primRead: fd count: anInteger
	<primitive: 'FileStream_read'>
	self primitiveFailed!

FileStream method!
primWrite: fd byte: anInteger
	<primitive: 'FileStream_write'>
	self primitiveFailed!

FileStream method!
primWrite: fd byteArray: aByteArray
	<primitive: 'FileStream_write'>
	self primitiveFailed!

FileStream method!
primClose: fd
	<primitive: 'FileStream_close'>
	self primitiveFailed!

FileStream method!
next
	"Read the next byte from the receiver"
	^ self primRead: fdesc count: 1!

FileStream method!
next: anInteger
	"Read the next anInteger bytes from the receiver"
	^ self primRead: fdesc count: anInteger!

FileStream method!
nextPut: anInteger
	"write anInteger to the receiver."
	^ self primWrite: fdesc byteArray: (ByteArray with: anInteger)!

FileStream method!
nextPutAll: aByteArray
	^ self primWrite: fdesc byteArray: aByteArray!


FileStream method!
close
	^ self primClose: fdesc!


FileStream method!
on: fd
	fdesc := fd!
	