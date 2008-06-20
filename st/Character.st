

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


Character classMethod!
lookup: code index: anInteger
	| l r m n offsets ranges |

	m := 0.
	n := anInteger.
	offsets := self offsets.
	ranges := self ranges.

	(l := offsets at: (n + 1)) == 16rFFFF
		ifTrue: [ ^ false ].

	[ ((m := m + 1) + n < offsets size)
	  & ((offsets at: (n + m + 1)) == 16rFFFF) ] whileTrue.

	r := (offsets at: (n + m + 1)) - 1.

	[ l <= r ]
		whileTrue: [
			m := (l + r) bitShift: -1.
			m := m - (m bitAnd: 1).
			code > (ranges at: (m + 1 + 1))
				ifTrue: [ l := m + 2 ]
				ifFalse: [ 
					code < (ranges at: (m + 1))
						ifTrue: [ r := m - 2 ]
						ifFalse: [
							((ranges at: (m + 1)) <= code) & (code <= (ranges at: (m + 1 + 1)))
								 ifTrue: [ ^ true ]]]].

	^ false!

Character classMethod!
isProp: code mask: anInteger
	| mask masks |

	mask := anInteger.
	masks := self mask.

	mask == 0
		ifTrue: [ ^ false ].

	0 to: 31 do: [ :i | 
		((mask bitAnd: (masks at: (i + 1))) ~~ 0) & (self lookup: code index: i)
			ifTrue: [ ^ true ]].

	^ false!

Character method!
isDigit
	^ self class isProp: self value
				 mask: 16r00000008!
