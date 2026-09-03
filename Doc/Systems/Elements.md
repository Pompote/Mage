# Éléments

Le framework élémentaire : statuts portés par les acteurs, affinités par type d'ennemi, et
résolution des réactions quand deux éléments différents se rencontrent. Consommé à la fois par les
sorts (`SpellCasting.md`) et par les ennemis/le grab (`EnemiesAndGrab.md`) — c'est pour ça qu'il vit
dans son propre dossier plutôt que dans `Spells/` (le mettre dans `Spells/` aurait inversé la
dépendance réelle).

**Statut** : implémenté — voir README "Mécanique #2 : Éléments et interactions" pour le design/les
décisions de haut niveau ; ce fichier couvre le "comment c'est construit".

## Pièces

| Fichier | Rôle | Notes |
|---|---|---|
| `ElementTypes.h` | `EElement` (None/Fire/Water/Earth/Air) | `UENUM` C++ figé, pas une liste extensible par contenu — voir Points ouverts |
| `ElementalStatusComponent.h/.cpp` | Élément porté par un acteur, durée limitée | Point d'entrée unique pour "un élément touche cet acteur" |
| `ElementalAffinityProfile.h/.cpp` | Data Asset : comportement d'un type d'ennemi par élément | `Normal` / `Absorbent` + multiplicateur de durée |
| `ElementReactionTable.h/.cpp` | Data Asset unique : 6 lignes de réaction | Lookup non ordonné (Feu+Eau = Eau+Feu) |
| `ElementReactionSubsystem.h/.cpp` | Point d'entrée unique (`GameInstanceSubsystem`) | Résout une réaction, spawn l'effet correspondant |
| `ElementReactionEffectActor.h/.cpp` | Placeholder générique du résultat visuel d'une réaction | Sphère de debug ; à remplacer par un vrai mesh/Niagara par ligne plus tard |
| `ElementFunctionLibrary.h/.cpp` | `DeliverElementToActor` : point d'entrée partagé "livrer un élément au contact" | Utilisé par `ASpellProjectile::OnHit` et par le jet (grab/lancer) |

## UElementalStatusComponent

- **Rôle** : `ApplyElement(NewElement, SourceActor, ContactLocation)` est **le seul endroit** où
  "un élément touche cet acteur" est traité, quelle que soit la source (sort ou porteur jeté) :
  - aucun élément présent → pose le statut, durée = `BaseElementDurationSeconds` × multiplicateur
    d'affinité (voir `UElementalAffinityProfile`)
  - même élément déjà présent → rafraîchit juste la durée, ne rediffuse **pas** `OnElementChanged`
    (l'état visible ne change pas) — propriété utile : ça empêche aussi une éventuelle boucle de
    propagation (A repasse à B qui repasse à A) de s'emballer, sans logique de garde dédiée
  - élément **différent** déjà présent → déclenche la réaction via `UElementReactionSubsystem`,
    puis efface le statut des deux côtés
- **Décision de design** : expiration par `FTimerHandle`, pas par `Tick()` — pas besoin de
  précision frame par frame pour un statut à durée de plusieurs secondes.
- **`OnElementChanged`** (délégué `BlueprintAssignable`) : diffusé à chaque changement réel
  d'élément (application, expiration, remise à None après réaction) — jamais sur un simple
  rafraîchissement de durée. Sert à teinter un mesh (voir `EnemiesAndGrab.md` pour l'exemple
  `AMageEnemyCharacter`) sans avoir à interroger l'état chaque frame.

## UElementalAffinityProfile

- **Rôle** : `TArray<FElementAffinityEntry>` — une entrée par élément pertinent
  (`Element`, `Behavior` : `Normal`/`Absorbent`, `DurationMultiplier`). Un élément absent de la
  liste se comporte comme `Normal` / x1.0 par défaut — pas un cas particulier à coder ailleurs,
  juste l'absence d'une ligne. Nouveau type d'ennemi = nouvel asset, jamais de branchement C++ par
  type (voir `Doc/WORKFLOW.md`).
- **`Absorbent`** : durée nettement prolongée — mauvais choix pour affaiblir directement cet
  ennemi, mais un excellent "réservoir" à saisir puis lancer ailleurs (ex : le golem d'argile
  imprègne l'eau et le feu, voir `DA_AffinityGolem`).

## UElementReactionTable / UElementReactionSubsystem

- **Rôle** : `UElementReactionTable` est un Data Asset unique (pas de découverte automatique
  façon formes de sorts — une seule table existera jamais) avec exactement 6 lignes pour
  {Fire, Water, Earth, Air}. `FindReaction(A, B)` fait un lookup non ordonné (parcourt le tableau,
  matche dans les deux sens). `UElementReactionSubsystem` (même précédent que
  `USpellRecognitionSubsystem`, voir `GestureRecognition.md`) charge la table une fois par session
  (chemin dans `Config/DefaultGame.ini`, section `[/Script/Mage.ElementReactionSubsystem]`),
  résout une réaction et spawn `ReactionEffectClass` à l'endroit du contact.
- **Ce qui reste volontairement simple** : les 6 réactions pointent aujourd'hui vers un seul
  acteur placeholder partagé (`AElementReactionEffectActor`, sphère de debug) plutôt que 6
  mécaniques de gameplay distinctes — la boucle complète (appliquer, faire se rencontrer, réagir,
  saisir/lancer) était la priorité à valider avant de peaufiner chaque résultat individuellement.

## UElementFunctionLibrary

- **Rôle** : `DeliverElementToActor(Element, TargetActor, SourceActor, ContactLocation)` — trouve
  le `UElementalStatusComponent` de `TargetActor` (s'il existe) et appelle `ApplyElement`. Ne fait
  rien (retourne `false`) si l'élément est `None`, si `TargetActor` est nul, ou si la cible n'a pas
  de composant élémentaire — ce dernier cas n'est pas une erreur, juste un non-événement.
  Centralise "comment je livre un élément au contact" pour ne l'écrire qu'une seule fois, plutôt
  que dupliquer la logique par type de porteur (projectile vs jet).

## Points ouverts

- **États composés / graphe de réactions** : et si un résultat de réaction (Steam, Mud...)
  devenait lui-même un état qu'un acteur peut porter et qui peut réagir à son tour (Steam + Ice →
  Water, par exemple) — un vrai graphe d'états plutôt qu'un simple tableau de paires ?
  Impliquerait de remplacer `EElement` (enum C++ figé) par une identité d'état extensible par
  contenu (façon `FName`, comme les ID de formes) pour les états composés, sans recompiler à chaque
  nouvel état. Idée en réflexion, pas encore tranchée.

## Renvois

- `SpellDefinition::Element`, livraison à l'impact d'un projectile → `SpellCasting.md`
- Porté par `AMageEnemyCharacter`, livré à l'impact d'un jet → `EnemiesAndGrab.md`
