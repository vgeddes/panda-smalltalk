

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
at: anInteger put: aCharacter
	<primitive: 'ByteString_at_put'>
	aCharacter isCharacter
		ifFalse: [self error: 'object is not a character'].
	anInteger isInteger
		ifTrue: [self error: 'index out of bounds'].
	anInteger isNumber
		ifTrue: [^ self at: anInteger asInteger]
		ifFalse: [self error: 'index is not an integer']!


ByteString method!
size
	<primitive: 'ByteString_size'>
	self primitiveFailed!

"comparing"

ByteString method!
compare: aString
	<primitive: 'ByteString_compare'>
	self error: 'argument is not a ByteString'!

ByteString method!
hash
	<primitive: 'ByteArray_hash'>
	self primitiveFailed!

"testing"

ByteString method!
isByteString
	^ true!