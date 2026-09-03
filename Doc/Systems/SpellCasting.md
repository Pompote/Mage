# Sorts : buffer et cast (résolution)

Ce qui se passe une fois qu'une forme est reconnue : la placer dans une banque temporaire, puis la
caster réellement sur une touche dédiée. Ne sait rien de comment une forme est reconnue — c'est la
face "gameplay" de la frontière décrite dans `GestureRecognition.md`.

**Statut** : implémenté (buffer, cast, premier effet de sort réel)

**Principe directeur** : la reconnaissance ne doit jamais savoir ce qu'est un sort, et le gameplay
ne doit jamais savoir comment une forme est reconnue. Tant que cette frontière tient, chaque côté
évolue indépendamment (nouvel algo, nouvelles formes, règles de composition Phase 2) sans onde de
choc sur le reste.

## Pièces

| Fichier | Rôle | Notes |
|---|---|---|
| `SpellBufferComponent.h/.cpp` | Banque FIFO de sorts préparés, cast sur touche dédiée | S'abonne à `OnGestureRecognized` de `SpellDrawComponent` |
| `SpellDefinition.h` | Data Asset : forme → sort | `Shape`, `Icon`, `DisplayName`, `EffectClass`, `Element` |
| `SpellEffect.h/.cpp` | Classe de base des effets de sort | Ne fait rien par défaut ; point d'extension central |
| `ProjectileSpellEffect.h/.cpp` | Effet générique : fait apparaître un projectile | Aucun code par nouveau sort-projectile, juste un Blueprint |
| `SpellProjectile.h/.cpp` | Acteur projectile générique | Vole tout droit, se détruit au premier impact, livre son élément |
| `SpellCastContext.h` | `FSpellCastContext` (Caster, Element, SpellDefinition) | Contexte passé à `Execute` |

## USpellBufferComponent

- **Rôle** : reçoit les formes reconnues, cherche une correspondance dans `KnownSpells` (liste
  manuelle, pas de découverte automatique — contrairement aux formes elles-mêmes) via
  `Spell->Shape->ShapeId == Result.TemplateId`, pousse dans un buffer FIFO à `MaxBufferSize`
  emplacements (3 par défaut, ajustable sans recompiler). `CastSlot(int32)` instancie
  `Spell->EffectClass`, construit un `FSpellCastContext`, appelle `Effect->Execute(Context)`, puis
  retire l'emplacement.
- **Cast des emplacements (touches 1/2/3)** : une seule Input Action (`IA_CastSpell`, Axis1D/float)
  sert les 3 touches plutôt que 3 Input Actions distinctes — chaque touche porte son propre
  modificateur *Scalar* (1.0/2.0/3.0) dans l'IMC, ce qui permet de retrouver l'emplacement visé à
  partir de la seule valeur reçue côté C++ (`AMageCharacter::CastSlot`, voir `Character.md`).
  Augmenter `MaxBufferSize` au-delà de 3 ne demande qu'une touche de plus dans l'IMC (Scalar 4.0),
  aucune recompilation.
- **Lien réseau** : la reconnaissance reste 100% côté client (pas de réplication du tracé brut) ;
  seule cette couche de résolution enverra plus tard une requête au serveur ("je veux lancer le
  sort X"), qui valide et exécute — cohérent avec le principe "client demande, serveur décide"
  retenu pour la coop. Pas encore implémenté (rien n'est répliqué dans le projet à ce stade).

## USpellEffect / FSpellCastContext

- **Décision de design clé** : `Execute` prend un `FSpellCastContext` (Caster + Element +
  SpellDefinition) plutôt qu'un simple `AActor* Caster`. Changement obligatoire à un moment donné
  (un `UFUNCTION BlueprintNativeEvent` ne peut pas être surchargé par UHT), donc plutôt qu'un
  paramètre isolé à recasser au prochain besoin, un struct extensible absorbe les futurs ajouts de
  contexte sans re-casser la signature (et donc sans avoir à reconnecter tout override Blueprint
  existant).

## ASpellProjectile / UProjectileSpellEffect

- **Rôle** : `UProjectileSpellEffect` fait apparaître un `ASpellProjectile` devant la caméra du
  lanceur, dans sa direction de visée **au moment du cast** (pas du dessin — cohérent avec le
  principe du buffer : viser/déclencher se décide au cast). `ASpellProjectile` porte un `Element`
  (assigné par l'effet à l'instanciation) et le livre via `UElementFunctionLibrary::DeliverElementToActor`
  à l'impact (voir `Elements.md`).
- **Gotcha / bug trouvé** : `"Projectile"` n'est **pas** un préréglage de collision qui existe
  réellement (ni dans ce projet, ni dans le moteur) — `SetCollisionProfileName("Projectile")`
  échouait silencieusement, laissant le rocher sur un état de collision par défaut qui ne bloquait
  rien contre les autres acteurs. Remplacé par `BlockAllDynamic`, un profil qui existe vraiment.
  Voir aussi `bNotifyRigidBodyCollision = true` ajouté par prudence sur le composant de collision.

## Renvois

- Réception des formes reconnues → `GestureRecognition.md`
- `Element` porté par un sort/projectile, `DeliverElementToActor` → `Elements.md`
- Le sort de manipulation (grab/lancer) est un système à part, toujours actif, pas dans ce
  buffer → `EnemiesAndGrab.md`
