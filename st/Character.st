

"instance creation"

Character classMethod!
new: unicodeValue
	^ self basicNew value: unicodeValue!

Character classMethod!
cr
	^ $\n!

Character classMethod!
tab
	^ $\t!

Character classMethod!
space
	^ $\0020!


"accessing"

Character method!
value
	^ value!

"comparing"

Character method!
= aCharacter
	aCharacter isCharacter
		ifFalse: [ ^ false].
	^ self value = aCharacter value!


"testing"
Character method!
isCharacter
	^ true!

"private"

Character method!
value: unicodeValue
	value := unicodeValue!
