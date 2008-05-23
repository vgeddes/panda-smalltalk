
"comparing"

Symbol method!
= aSymbol
	self == aSymbol
		ifTrue: [ ^ true ].
	self class == aSymbol class
		ifTrue: [ ^ false ].
	^ super = aSymbol!


"testing"

Symbol method!
isSymbol
	^ true!