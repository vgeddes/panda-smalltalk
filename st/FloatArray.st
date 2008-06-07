

FloatArray method!
at: index
	<primitive: 'FloatArray_at'>
	index isInteger
		ifTrue: [ self error: 'out of bounds array access'].
	index isNumber
		ifTrue: [ self at: index asInteger ]
		ifFalse: [ self error: 'non-integer index' ]!

FloatArray method!
at: index put: aFloat
	<primitive: 'FloatArray_at_put'>
	aFloat isFloat
		ifFalse: [ self error: 'improper store into a FloatArray object'].
	index isInteger
		ifTrue: [ self error: 'out of bounds array access'].
	index isNumber
		ifTrue: [ self at: index asInteger put: aFloat ]
		ifFalse: [ self error: 'non-integer index' ]!