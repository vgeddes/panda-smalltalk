
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

"methods"

Behavior method!
addSelector: aSymbol withMethod: aMethod
	methodDictionary at: aSymbol put: aMethod!

Behavior method!
removeSelector: aSymbol
	methodDictionary removeKey: aSymbol!

Behavior method!
selectors
	^ methodDictionary keys!

Behavior method!
allSelectors
	| class selectors |

	selectors := Set new.

	class := self.
	[class ~~ nil]
		whileTrue:
			[selectors includeAll: class selectors.
			 class := class superclass].

	^ selectors!

Behavior method!
hasMethods
	^ methodDictionary isEmpty not!

Behavior method!
includesSelector: aSymbol
	^ methodDictionary includes: aSymbol!

Behavior method!
canUnderstand: aSymbol
	| class |

	class := self.
	[class ~~ nil]
		whileTrue:
			[(methodDictionary includesKey: aSymbol) ifTrue: [^ true].
			 class := class superclass].

	^ false!

Behavior method!
inheritsFrom: aClass
	| class |

	class := self.
	[class ~~ nil]
		whileTrue:
			[class == aClass ifTrue: [^ true].
			 class := class superclass].

	^ false!


"compilation"

Behavior method!
compile: aString
	<primitive: 'Behavior_compile'>
	self error: 'could not compile code'!

"printing"

Behavior method!
printOn: aStream
	aStream nextPutAll: self name!


	