

"instance creation"

Character classMethod!
characterFor: unicodeValue
	<primitive: 'Character_characterFor'>
	self primitiveFailed!

Character classMethod!
new
	^ self basicNew!

Character classMethod!
new: anInteger
	^ self basicNew!

Character classMethod!
basicNew
	self error: 'Characters can only be created with #characterFor:'!


"common integers"

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
	<primitive: 'Character_value'>
	self primitiveFailed!

"comparing"

Character method!
= aCharacter
	^ aCharacter isCharacter and: [(self compare: aCharacter) = 0]!

Character method!
< aCharacter
	^ (self compare: aCharacter) < 0!

Character method!
> aCharacter
	^ (self compare: aCharacter) > 0!

Character method!
<= aCharacter
	^ (self compare: aCharacter) <= 0!

Character method!
>= aCharacter
	^ (self compare: aCharacter) >= 0!

Character method!
compare: aCharacter
	^ self value - aCharacter value!

Character method!
hash
	^ self value!


"testing"

Character method!
isCharacter
	^ true!

