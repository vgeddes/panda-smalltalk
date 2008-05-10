
"evaluation"

BlockContext method!
value
	<primitive: 'BlockContext_value'>
	self primitiveFailed!
Annotation key: 'category' value: 'evaluation'!

BlockContext method!
value: argument
	<primitive: 'BlockContext_valueColon'>
	self primitiveFailed!
Annotation key: 'category' value: 'evaluation'!

BlockContext method!
value: firstArgument value: secondArgument
	<primitive: 'BlockContext_value_value'>
	self primitiveFailed!
Annotation key: 'category' value: 'evaluation'!

BlockContext method!
value: firstArgument value: secondArgument value: thirdArgument
	<primitive: 'BlockContext_value_value_value'>
	self primitiveFailed!
Annotation key: 'category' value: 'evaluation'!

BlockContext method!
valueWithArguments: anArray
	<primitive: 'BlockContext_valueWithArguments'>
	self primitiveFailed!
Annotation key: 'category' value: 'evaluation'!

BlockContext method!
valueWithArguments: anArray
	<primitive: 'BlockContext_valueWithArguments'>
	self primitiveFailed!
Annotation key: 'category' value: 'evaluation'!


"controlling"

BlockContext method!
whileTrue
	^ [self value] whileTrue: [nil]!
Annotation key: 'category' value: 'controlling'!

BlockContext method!
whileFalse
	^ [self value] whileFalse: [nil]!
Annotation key: 'category' value: 'controlling'!

BlockContext method!
whileTrue: aBlock
	^ [self value] whileTrue: [aBlock value]!
Annotation key: 'category' value: 'controlling'!

BlockContext method!
whileFalse: aBlock
	^ [self value] whileFalse: [aBlock value]!
Annotation key: 'category' value: 'controlling'!