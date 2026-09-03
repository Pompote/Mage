# Workflows

Guides pratiques, étape par étape, pour ajouter ou modifier du contenu dans le jeu. Le README garde
la vision, le statut d'implémentation et les décisions de design — ce fichier garde le "comment
faire", pour éviter de le redire à deux endroits.

## Compiler après un changement (note transversale)

- **Live Coding** (`Ctrl+Alt+F11`, éditeur ouvert) suffit si le changement ne touche que des corps
  de fonction (pas de nouveau/modifié `UCLASS`/`USTRUCT`/`UENUM`/`UPROPERTY`/`UFUNCTION`).
- **Rebuild complet** (éditeur fermé) obligatoire dès qu'un de ces éléments réfléchis change.
  Commande directe :
  ```
  "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" MageEditor Win64 Development "D:\code\unreal\jeux\Mage\Mage.uproject" -waitmutex
  ```
- Deux pièges déjà rencontrés à garder en tête : un changement de constructeur passé par Live
  Coding ne rafraîchit pas toujours le CDO déjà chargé (le "reset to default" d'un Blueprint peut
  rester bloqué sur l'ancien état tant qu'un vrai rebuild n'a pas eu lieu) ; une valeur assignée à la
  main dans un Blueprint (matériau, mesh, collision...) peut elle aussi rester "figée" sur une
  ancienne valeur indépendamment du C++ — regarder la petite flèche jaune "reset to default" à côté
  du champ concerné en cas de comportement qui ne correspond pas au code.

---

## 1. Ajouter une nouvelle forme de geste (shape)

Purement data, aucun code.

1. Fournir une image (fond clair, trait sombre) dans `Content/Spells/Shapes/SourceImages/`, nommée
   comme l'ID de forme voulu (ex : `Losange.png` → `ShapeId = "Losange"`).
2. Ouvrir `SpellGenerator` (`UGestureShapeBatchGenerator`, `Content/Spells/Shapes/`) → bouton
   **Generate Missing Shape Templates**. Crée et bake automatiquement un `GST_Losange`
   (`UGestureShapeTemplate`) pour toute image pas encore utilisée.
3. Si la reconnaissance est trop stricte/permissive pour cette forme précise : régler
   `RecognitionThresholdOverride` sur le `GST_*` généré (sinon le seuil global s'applique).

## 2. Ajouter un nouveau sort

Prérequis : une forme existe (`GST_X`, voir workflow 1).

1. Créer `SD_X` (`USpellDefinition`, `Content/Spells/Definition/`) : `Shape = GST_X`, `Icon` (texture
   importée dans `Content/Spells/Icons/`), `DisplayName`, `Element` si le sort doit porter un
   élément (optionnel, `EElement`).
2. Choisir l'effet :
   - **Réutiliser un projectile existant** : créer `BP_XProjectile` (Blueprint enfant de
     `ASpellProjectile`, assigner mesh + matériau), puis `BP_XSpellEffect` (Blueprint enfant de
     `UProjectileSpellEffect`, `Projectile Class = BP_XProjectile`) — aucun code.
   - **Effet fondamentalement différent** (pas un projectile) : nouvelle classe C++ fille de
     `USpellEffect`, override `Execute_Implementation(const FSpellCastContext& Context)`.
3. Assigner `SD_X.EffectClass = BP_XSpellEffect` (ou l'équivalent C++).
4. Ajouter `SD_X` à **Known Spells** sur le `SpellBufferComponent` de `BP_MageCharacter`.

Aucune compilation si tout reste en Blueprint/Data Asset (étapes 1, 2a, 3, 4) ; rebuild complet
uniquement si une nouvelle classe C++ est créée (2b).

## 3. Ajouter un élément

Le seul des quatre qui touche vraiment au code : `EElement` (`Source/Mage/Elements/ElementTypes.h`)
est un `UENUM` C++ figé, pas une liste extensible par contenu.

1. Ajouter la valeur dans l'enum `EElement`.
2. Mettre à jour tout `switch`/`Switch on EElement` exhaustif qui existe déjà (aujourd'hui :
   `AMageEnemyCharacter::HandleElementChanged`, et tout futur Switch on EElement ajouté en
   Blueprint pour une teinte/VFX) pour couvrir le nouveau cas.
3. Ajouter les lignes de réaction correspondantes dans `DA_ElementReactionTable` pour chaque paire
   impliquant ce nouvel élément avec les éléments déjà existants (voir workflow 5).
4. Rebuild complet (nouvelle valeur d'enum = régénération UHT).
5. Assigner le nouvel élément aux sorts (`SD_X.Element`) et vérifier les profils d'affinité
   concernés (workflow 4) si certains types d'ennemis doivent réagir spécifiquement à lui.

Voir le "point ouvert" du README (Mécanique #2) sur l'idée de rendre les états eux-mêmes
extensibles par contenu (façon ID de formes) plutôt que de repasser par cette étape à chaque fois —
pas encore tranché.

## 4. Ajouter une affinité (profil élémentaire pour un type d'ennemi)

Purement data, aucun code.

1. Créer `DA_Affinity_X` (`UElementalAffinityProfile`, `Content/Enemies/Affinities/`).
2. Ajouter une entrée par élément pertinent : `Element`, `Behavior` (`Normal` ou `Absorbent`),
   `DurationMultiplier` (1.0 = durée standard, >1.0 = porteur longue durée).
3. Assigner sur le composant **Elemental Status** de l'ennemi concerné (`Affinity Profile`).

Un élément absent de la liste se comporte comme `Normal` / x1.0 par défaut — pas besoin de lister
tous les éléments si seuls certains ont un comportement particulier.

## 5. Ajouter une nouvelle réaction (résultat d'une paire d'éléments)

Purement data si les deux éléments existent déjà (voir workflow 3 sinon).

1. Ouvrir `DA_ElementReactionTable` (`Content/Elements/`) → ajouter une ligne : `Element A`,
   `Element B`, `Reaction Id` (nom cosmétique, ex `Steam`), `Reaction Effect Class`.
2. Par défaut, réutiliser `BP_ElementReactionPlaceholder` (sphère de debug partagée). Pour un vrai
   effet dédié : Blueprint enfant de `AElementReactionEffectActor` (visuel/tunable sans code), ou
   nouvelle classe C++ si la réaction a une vraie logique de gameplay (dégâts, ralentissement...).

## 6. Ajouter un nouvel ennemi

**Cas simple** (marche vers le joueur, juste une nouvelle affinité/apparence) — aucun code :

1. Créer ou choisir un profil d'affinité (workflow 4).
2. Créer `BP_X` (Blueprint enfant de `AMageEnemyCharacter`).
3. Assigner **Affinity Profile** et **Base Tint Material** sur les composants concernés.
4. Régler **Walk Speed**, **Stop Distance**, **Thrown Flight Speed** selon le comportement voulu.
5. Remplacer le mesh de **Placeholder Mesh** par un vrai visuel si disponible (sinon garder la
   capsule/forme de base du moteur).
6. Placer dans le niveau.

**Cas complexe** (IA différente, attaque, PV, télégraphe au sol type "pierre-feuille-ciseaux"...) :
demande une nouvelle classe C++ (sous-classe de `AMageEnemyCharacter` ou `ACharacter` directement
selon le besoin). Pas encore de pattern établi pour ça — à construire au cas par cas quand un
premier ennemi "de combat" sera attaqué.

## 7. Ajouter ou modifier une touche (Input Action)

Récurrent dans presque tous les workflows ci-dessus dès qu'une nouvelle action joueur apparaît.

1. Créer `IA_X` (Input Action, Content Browser → Input).
2. L'ajouter à `IMC_Default` avec la ou les touches voulues (+ modificateurs si besoin, ex : Scalar
   pour distinguer plusieurs touches sur une même action, voir `IA_CastSpell`).
3. Si l'action doit déclencher un nouveau comportement (pas juste réutiliser une action existante) :
   ajouter un champ `UPROPERTY(EditDefaultsOnly) TObjectPtr<UInputAction>` sur la classe C++
   concernée + le binder dans `SetupPlayerInputComponent` — **nécessite du C++ et un rebuild
   complet**.
4. Assigner `IA_X` au champ correspondant sur le Blueprint (`BP_MageCharacter` en général).
