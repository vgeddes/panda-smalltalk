
"instance creation"

Behavior method!
new
	^ self basicNew initialize!
Annotation key: 'category' value: 'instance creation'!

Behavior method!
new: sizeRequested
	^ (self basicNew: sizeRequested) initialize!
Annotation key: 'category' value: 'instance creation'!

Behavior method!
basicNew
	<primitive: 'Behavior_new'>
	self primitiveFailed!
Annotation key: 'category' value: 'instance creation'!

Behavior method!
basicNew: anInteger
	<primitive: 'Behavior_newSize'>
	self primitiveFailed!
Annotation key: 'category' value: 'instance creation'!


"accessing"

Behavior method!
format
	^ format!
Annotation key: 'category' value: 'accessing'!

Behavior method!
superclass
	^ superclass!
Annotation key: 'category' value: 'accessing'!

Behavior method!
instanceSize
	^ instanceSize!
Annotation key: 'category' value: 'accessing'!

Behavior method!
methodDictionary
	^ methodDictionary!
Annotation key: 'category' value: 'accessing'!

Behavior method!
instanceVariableNames
	^ instanceVariableNames!
Annotation key: 'category' value: 'accessing'!