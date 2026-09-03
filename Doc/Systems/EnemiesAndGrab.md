# Ennemis et manipulation (grab + lancer)

L'ennemi de test (marche vers le joueur, porte un élément) et le sort de manipulation toujours
actif qui permet de le saisir et de le lancer. Réunis dans un même fichier : les deux dossiers sont
petits et ne se comprennent bien qu'ensemble — l'état `Thrown` de l'ennemi n'existe qu'à cause de
`UGrabThrowComponent`, et la seule raison d'être du grab est de manipuler des acteurs
`IGrabbableInterface`.

**Statut** : implémenté (ennemi de test minimal, saisie/portage/lancer, jalon de bout en bout
validé — voir README "Mécanique #2")

## Pièces

| Fichier | Rôle | Notes |
|---|---|---|
| `MageEnemyCharacter.h/.cpp` | Ennemi de test (`ACharacter`) | Marche vers le joueur, pas de PV/combat, porte un élément |
| `GrabThrowComponent.h/.cpp` | Sort de manipulation, sur `AMageCharacter` | Toujours disponible, pas dessiné, pas dans le buffer |
| `GrabbableInterface.h/.cpp` | Interface "peut être saisi/porté/lancé" | Implémentée par `AMageEnemyCharacter` |

## AMageEnemyCharacter

- **Rôle** : ennemi de test minimal — marche vers le joueur le plus proche (`TickSeeking`,
  `AddMovementInput` simple, pas de NavMesh/AIController), pas de PV/combat, juste un profil
  élémentaire (`ElementalStatus` + `UElementalAffinityProfile` assigné dans l'éditeur). Sert à
  jouer la boucle grab → porter → lancer en PIE, pas à préfigurer le vrai roster d'ennemis.
- **Machine à états** (`EEnemyMotionState`) : `Seeking` (comportement par défaut) → `Held` (porté,
  `Tick()` ne fait rien, c'est `UGrabThrowComponent` qui repositionne cet acteur chaque frame) →
  `Thrown` (`TickThrown` avance et détecte l'impact).
- **`TickThrown`** réutilise le motif déjà en place pour le dash du joueur
  (`AddActorWorldOffset` balayé chaque frame, voir `Character.md`) plutôt que
  `UProjectileMovementComponent`, qui entrerait en conflit avec le `UCharacterMovementComponent`
  d'un `ACharacter`. À l'impact : livre son élément via `UElementFunctionLibrary::DeliverElementToActor`
  puis efface son propre statut (`ElementalStatus->ClearElement()`) — l'élément "part" du porteur
  qu'une réaction se soit déclenchée ou non sur la cible.
- **Teinte du mesh** : `BaseTintMaterial` (assigné dans l'éditeur, ex. `M_ElementTint`) sert de
  base à un `UMaterialInstanceDynamic` créé en code (`BeginPlay`), piloté par
  `HandleElementChanged` (abonné à `ElementalStatus->OnElementChanged`).
  - **Gotcha rencontré** : ce chemin passe par le C++ plutôt que par une assignation manuelle du
    matériau dans le Details panel du composant `Placeholder Mesh` — l'UI d'assignation directe
    ("Override Materials") s'est avérée peu fiable pendant les tests (voir gotcha général
    ci-dessous). Exposer `BaseTintMaterial` comme un simple champ `EditDefaultsOnly` (comme
    `AffinityProfile` ou `ProjectileClass` ailleurs dans le projet) a été plus robuste que de
    charger le matériau par chemin en dur dans le constructeur.

## UGrabThrowComponent / IGrabbableInterface

- **Rôle** : `IGrabbableInterface` (`CanBeGrabbed`/`OnGrabbed`/`OnThrown`) est une interface plutôt
  qu'une base commune avec `ASpellProjectile`, pour que n'importe quel type d'acteur puisse
  l'implémenter sans hériter d'une classe partagée. `UGrabThrowComponent` (sur `AMageCharacter`) :
  `TryGrab()` fait un sphere trace sur le canal **Pawn** depuis la caméra, cast vers l'interface ;
  tant qu'un acteur est tenu, `TickComponent` le repositionne devant la caméra chaque frame ;
  `ReleaseOrThrow()` délègue à l'acteur saisi via `Execute_OnThrown` (le composant ne gère que la
  visée, il ne pilote pas le vol — même séparation que `UProjectileSpellEffect` → `ASpellProjectile`,
  voir `SpellCasting.md` : celui qui lance ne gère que la visée, celui qui vole gère son propre
  impact).
- Le trace de saisie utilise le canal `ECC_Pawn` existant plutôt qu'un nouveau canal de collision
  dédié — suffisant tant que seuls les ennemis sont saisissables.

## Gotcha général (rencontré deux fois pendant les tests)

Une valeur assignée manuellement dans un Blueprint (matériau, mesh, préréglage de collision...)
peut rester "figée" sur une ancienne valeur indépendamment du C++, et un changement de constructeur
appliqué par Live Coding ne rafraîchit pas toujours le CDO déjà chargé dans l'éditeur en cours de
session — "reset to default" peut continuer à revenir sur l'ancien état tant qu'un vrai rebuild
complet (éditeur fermé) n'a pas forcé la reconstruction du CDO. Voir la note transversale en tête
de `Doc/WORKFLOW.md`.

## Renvois

- `ElementalStatus`, `UElementalAffinityProfile` → `Elements.md`
- Le sort de manipulation est branché sur `AMageCharacter` (composant `GrabThrowComponent`,
  Input Action `GrabThrowAction`) → `Character.md`
