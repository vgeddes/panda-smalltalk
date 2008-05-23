

StringStream classMethod!
on: aCollection
	^ self notYetImplemented!


"accessing"

StringStream method!
nextPut: aCharacter
	| value |

	aCharacter isCharacter
		ifFalse: [ ^ self error: 'StringStream only supports Character objects'].

	super nextPut: aCharacter value!

StringStream method!
nextPutAll: aString
	| value |

	aString isString
		ifFalse: [ self error: 'StringStream only supports Character objects'].

	super nextPutAll: aString bytes!

StringStream method!
show: aString
	self nextPutAll: aString!

StringStream method!
contents
	^ String newFromBytes: collection!


"private"

StringStream method!
initialize
	self on: (ByteArray new: 20)!