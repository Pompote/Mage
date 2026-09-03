# Personnage

Le personnage joueur (déplacement, caméra FPS, saut, dash) et le HUD associé — le "shell" auquel
tous les autres systèmes (dessin de sorts, buffer, éléments, grab/lancer) se connectent en tant que
composants ou lectures directes.

**Statut** : implémenté

## Pièces

| Fichier | Rôle | Notes |
|---|---|---|
| `MageCharacter.h/.cpp` | Le personnage joueur (`ACharacter`) | Caméra FPS, Enhanced Input, dash, héberge tous les composants de sorts/éléments |
| `MageHUD.h/.cpp` | Affichage HUD | Tracé de dessin, icônes du buffer, cooldown du dash — purement visuel |
| `Mage.Build.cs`, `Mage.h/.cpp` | Boilerplate du module UBT | Rien de spécifique au jeu |

## AMageCharacter

- **Rôle** : hérite de `ACharacter` (pas `APawn` brut) pour la gestion gratuite du sol/gravité/saut
  et la réplication réseau native de `CharacterMovementComponent` — sert directement l'objectif
  "prêt pour la coop". Caméra attachée directement à la capsule (pas de spring arm), positionnée à
  `BaseEyeHeight` ; `bUseControllerRotationYaw = true` (le corps suit le regard) ;
  `bOrientRotationToMovement = false` (pas d'auto-orientation, comportement FPS standard).
- **Composants hébergés** : `SpellDrawComponent`, `SpellBufferComponent`, `GrabThrowComponent` —
  `AMageCharacter` ne fait que les créer et leur router l'input ; voir `GestureRecognition.md`,
  `SpellCasting.md`, `EnemiesAndGrab.md` pour ce qu'ils font réellement.
- **Dash** : `DashSpeed` (unités/seconde) et `DashRange` (unités) sont volontairement deux réglages
  séparés — `DashRange` détermine seul si un dash suffit à esquiver une attaque donnée,
  `DashSpeed` ne contrôle que le côté "instantané" du mouvement (durée du dash = `DashRange /
  DashSpeed`). Exécuté dans `Tick()` via un `AddActorWorldOffset` balayé (sweep) chaque frame,
  indépendamment du dessin en cours. Dasher **annule un dessin en cours**
  (`SpellDrawComponent::CancelStroke()`) plutôt que de le laisser survivre — décision volontaire :
  sinon le dash deviendrait un moyen gratuit d'annuler un cast raté, ce qui tuerait la tension
  "dessiner expose" dont dépend le buffer de sorts.
- **Caméra pendant le dessin** : `CameraAttenuationWhileDrawing` (fraction de la sensibilité
  normale encore appliquée pendant le tracé — garde une sensation "le mage regarde autour de lui"
  plutôt qu'un blocage total), plus un tremblement croissant (`DrawShakeGrowthPerSecond` /
  `DrawShakeMaxAmplitude` / `DrawShakeFrequency`) et un micro-jitter à fréquence contrôlable
  (`DrawMicroJitterAmplitude` / `DrawMicroJitterFrequency`, une impulsion discrète par secousse,
  pas un bruit continu par frame).
- **Input** : Enhanced Input, un seul `IMC_Default`. `MoveAction`/`LookAction` (Axis2D),
  `JumpAction`/`DrawAction`/`DashAction`/`GrabThrowAction` (Digital). `CastSlotAction` est un cas
  particulier : une seule Input Action **Axis1D** partagée par les touches 1/2/3, chaque touche
  portant son propre modificateur *Scalar* (1.0/2.0/3.0) dans l'IMC — la valeur reçue dans
  `CastSlot()` indique directement quel emplacement caster. Avantage : augmenter la taille du
  buffer au-delà de 3 ne demande qu'une touche de plus dans l'IMC (Scalar 4.0), aucune
  recompilation.
- **Gotcha rencontré** : les bindings clavier doivent être dans **Default Key Mappings**, pas
  **Mapping Profile Overrides** (réservé à des profils de rebinding alternatifs, jamais actifs par
  défaut).
- **`DebugApplyElementAction` / `DebugApplyRandomElement()`** : outil de debug temporaire
  (marqué `@todo DEBUG A SUPPRIMER` dans le code), applique un élément aléatoire à ce que vise la
  caméra (trace sur le canal `ECC_Pawn` — `ECC_Visibility` ne fonctionne pas contre un `ACharacter`,
  dont le préréglage de collision "Pawn" ignore volontairement ce canal). Utile pour tester le
  système élémentaire sans passer par un vrai sort ; voir `Elements.md`.
- **GameMode** : `BP_MageGameMode` (parent `GameModeBase`), `Default Pawn Class = BP_MageCharacter`,
  réglé comme `GlobalDefaultGameMode` dans `Config/DefaultEngine.ini`. Niveau de test :
  `Content/Maps/Default/DefaultMap.umap`.

## AMageHUD

- **Rôle** : dessine le tracé en cours pendant le dessin d'un sort (croix de visée, ligne du tracé
  avec délai/bruit organique, mode écran ou monde 3D), la rangée d'icônes du buffer de sorts
  (bas-gauche), et l'indicateur de cooldown du dash (haut-gauche, se remplit de bas en haut, avec
  un flash à la consommation et au retour de disponibilité). Purement visuel — lit
  `SpellDrawComponent`/`SpellBufferComponent`/`AMageCharacter::GetDashCooldownRemainingFraction()`
  chaque frame, ne participe jamais à la reconnaissance ou au gameplay.
- **Gotcha** : le mode "tracé en espace monde" (`bDrawTraceInWorldSpace`) utilise
  `DrawDebugLine`/`DrawDebugSphere` — outils de debug, compilés hors des builds Shipping ; à
  remplacer par un vrai mesh/Niagara avant de sortir le jeu si ce mode est conservé.

## Renvois

- `SpellDrawComponent`, `SpellBufferComponent` → `Doc/Systems/GestureRecognition.md`,
  `Doc/Systems/SpellCasting.md`
- `GrabThrowComponent` → `Doc/Systems/EnemiesAndGrab.md`
- `DebugApplyRandomElement`, indicateur de teinte lu par le HUD → `Doc/Systems/Elements.md`
