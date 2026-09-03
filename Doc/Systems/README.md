# Référence technique

Documentation de développement : pour chaque système du jeu, ce qui existe en C++, comment ça
s'articule, les décisions de design et les bugs déjà rencontrés — au niveau de détail du code.
Complète, sans les redire, la vision/le statut du `README.md` principal et les procédures "comment
faire" de `Doc/WORKFLOW.md`.

- [Personnage](Character.md)
  - Pièces
  - AMageCharacter
  - AMageHUD
  - Renvois
- [Éléments](Elements.md)
  - Pièces
  - UElementalStatusComponent
  - UElementalAffinityProfile
  - UElementReactionTable / UElementReactionSubsystem
  - UElementFunctionLibrary
  - Points ouverts
  - Renvois
- [Ennemis et manipulation (grab + lancer)](EnemiesAndGrab.md)
  - Pièces
  - AMageEnemyCharacter
  - UGrabThrowComponent / IGrabbableInterface
  - Gotcha général (rencontré deux fois pendant les tests)
  - Renvois
- [Reconnaissance de formes (dessin de sorts)](GestureRecognition.md)
  - Pièces
  - USpellDrawComponent
  - UDollarOneRecognizer
  - UGestureShapeTemplate / UGestureShapeBatchGenerator
  - USpellRecognitionSubsystem
  - Points ouverts
  - Renvois
- [Sorts : buffer et cast (résolution)](SpellCasting.md)
  - Pièces
  - USpellBufferComponent
  - USpellEffect / FSpellCastContext
  - ASpellProjectile / UProjectileSpellEffect
  - Renvois
