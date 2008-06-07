
"accessing"

ByteArray method!
at: anInteger
	<primitive: 'ByteArray_at'>
	anInteger isInteger
		ifTrue: [self error: 'out of bounds array access'].
	anInteger isNumber
		ifTrue: [ self at: anInteger asInteger ]
		ifFalse: [self error: 'non-integer index']!

ByteArray method!
at: anInteger put: anObject
	<primitive: 'ByteArray_at_put'>
	anInteger isInteger
		ifTrue: [self error: 'out of bounds array access'].
	anInteger isNumber
		ifTrue: [ self at: anInteger asInteger ]
		ifFalse: [self error: 'non-integer index']!

ByteArray method!
hash
	<primitive: 'ByteArray_hash'>
	self primitiveFailed!