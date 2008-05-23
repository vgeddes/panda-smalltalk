
"instance creation"
ByteString classMethod!
newFromBytes: aByteArray
	^ (self basicNew: aByteArray size) initializeWithBytes: aByteArray!

"accessing"

ByteString method!
at: anInteger
	<primitive: 'ByteString_at'>
	anInteger isInteger
		ifTrue: [ self error: 'index out of bounds' ].
	anInteger isNumber
		ifTrue: [ ^ self at: anInteger asInteger ]
		ifFalse: [ self error: 'index is not an integer']!

ByteString method!
size
	<primitive: 'ByteString_size'>
	self primitiveFailed!

ByteString method!
byteAt: anInteger
	<primitive: 'ByteArray_at'>
	self primitiveFailed!

ByteString method!
byteAt: anInteger put: anObject
	<primitive: 'ByteArray_at_put'>
	self primitiveFailed!

ByteString method!
byteSize
	<primitive: 'ByteArray_size'>
	self primitiveFailed!

ByteString method!
bytes
	| bytes index |
	
	bytes := ByteArray new: self byteSize.

	index := 1.
	[ index <= bytes size ]
		whileTrue: [ bytes at: index put: (self byteAt: index).
					 index := index + 1].

	^ bytes!


"comparing"

ByteString method!
compare: aString
	<primitive: 'ByteString_compare'>
	self error: 'argument is not a ByteString'!

ByteString method!
compareOrdinal: aString
	"compare the individual Unicode code points in the string irrespective of locale"
	<primitive: 'ByteString_compareOrdinal'>
	self error: 'argument is not a ByteString'!

ByteString method!
hash
	<primitive: 'ByteString_hash'>
	self primitiveFailed!

"reversing"

ByteString method!
reversed
	<primitive: 'ByteString_reversed'>
	self primitiveFailed!
	

"testing"

ByteString method!
isByteString
	^ true!


"private"

ByteString method!
initializeWithBytes: aByteArray
	| index |

	index := 1.

	[ index <= aByteArray size ]
		whileTrue: [ self byteAt: index put: (aByteArray at: index).
					 index := index + 1]!

