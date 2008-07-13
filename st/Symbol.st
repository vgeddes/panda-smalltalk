
"comparing"

Symbol method!
= aSymbol
	self == aSymbol
		ifTrue: [ ^ true ].
	self class == aSymbol class
		ifTrue: [ ^ false ].
	^ super = aSymbol!


"accessing"

ByteSymbol method!
at: anInteger
	<primitive: 'ByteString_at'>
	anInteger isInteger
		ifTrue: [ self error: 'index out of bounds' ].
	anInteger isNumber
		ifTrue: [ ^ self at: anInteger asInteger ]
		ifFalse: [ self error: 'index is not an integer']!

ByteSymbol method!
size
	<primitive: 'ByteString_size'>
	self primitiveFailed!

"comparing"

ByteSymbol method!
compare: aString
	<primitive: 'ByteString_compare'>
	self error: 'argument is not a ByteString'!

ByteSymbol method!
hash
	<primitive: 'ByteArray_hash'>
	self primitiveFailed!

"testing"

Symbol method!
isSymbol
	^ true!

Symbol method!
printOn: aStream
	aStream nextPut: $#.
	aStream nextPutAll: self!
