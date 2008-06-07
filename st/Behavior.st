
"instance creation"

Behavior method!
new
	^ self basicNew initialize!

Behavior method!
new: size
	^ (self basicNew: size) initialize!

Behavior method!
basicNew
	<primitive: 'Behavior_new'>
	self primitiveFailed!

Behavior method!
basicNew: anInteger
	<primitive: 'Behavior_newSize'>
	self primitiveFailed!


"accessing"

Class method!
name
	^ name!

Metaclass method!
name
	^ instanceClass name, ' class'!

Behavior method!
format
	^ format!

Behavior method!
superclass
	^ superclass!

Behavior method!
instanceSize
	^ instanceSize!

Behavior method!
methodDictionary
	^ methodDictionary!

Behavior method!
instanceVariableNames
	^ instanceVariableNames!
