

WideString method!
at: anInteger
	<primitive: 'WideString_at'>
	anInteger isInteger
		ifTrue: [self error: 'index out of bounds'].
	anInteger isNumber
		ifTrue: [^ self at: anInteger asInteger]
		ifFalse: [self error: 'index is not an integer']!

WideString method!
at: anInteger put: aCharacter
	<primitive: 'WideString_at_put'>
	anInteger isInteger
		ifTrue: [self error: 'index out of bounds'].
	anInteger isNumber
		ifTrue: [^ self at: anInteger asInteger]
		ifFalse: [self error: 'index is not an integer'].
	aCharacter isCharacter
		ifFalse: [self error: 'object is not a character']!


