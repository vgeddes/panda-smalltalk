
"evaluation"

BlockContext method!
value
	<primitive: 'BlockContext_value'>
	self primitiveFailed!

BlockContext method!
value: argument
	<primitive: 'BlockContext_valueColon'>
	self primitiveFailed!

BlockContext method!
value: firstArgument value: secondArgument
	<primitive: 'BlockContext_value_value'>
	self primitiveFailed!

BlockContext method!
value: firstArgument value: secondArgument value: thirdArgument
	<primitive: 'BlockContext_value_value_value'>
	self primitiveFailed!

BlockContext method!
valueWithArguments: anArray
	<primitive: 'BlockContext_valueWithArguments'>
	self primitiveFailed!

BlockContext method!
valueWithArguments: anArray
	<primitive: 'BlockContext_valueWithArguments'>
	self primitiveFailed!


"controlling"

BlockContext method!
whileTrue
	^ [self value] whileTrue: [nil]!

BlockContext method!
whileFalse
	^ [self value] whileFalse: [nil]!

BlockContext method!
whileTrue: aBlock
	^ [self value] whileTrue: [aBlock value]!

BlockContext method!
whileFalse: aBlock
	^ [self value] whileFalse: [aBlock value]!

BlockContext method!
repeat
	^ [self value. true] whileTrue!


"error handling"

BlockContext method!
cannotReturn: anObject
	self error: 'cannot return object to a context that no longer exists'!
