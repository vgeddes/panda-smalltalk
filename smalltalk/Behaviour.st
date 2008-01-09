@package 'basic'

@class

Object subclass: #Behavior
       instanceVariableNames: 'name superclass instanceVariableNames instanceSize methodDictionary subclasses finalizationRequest'
       classVariableNames: ''
@

@category 'Accessing'

@method
allSubclasses
    "Answer a list of all subclasses of the receiver"
    | ret |
    ret := OrderedCollection new.
    self allSubclassesDo: [ :ea |
	ret add: ea ].
    ^ret
@
	
@def
allSuperclassesDo: aBlock  
    "Call aBlock for each superclass of this class"
    | class |
    class := self.
    [ (class := class superclass) notNil ]
	whileTrue: [ aBlock value: class ]
@

@category 'Doing this or that'


@



@

