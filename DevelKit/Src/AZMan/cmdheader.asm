
;********************************************************
;*
;*   Ce header d'objets de commande peut servir à tous
;*   les objets de commande. Il suffit de linker avec,
;*   pas besoin de copier le source à chaque fois pour
;*   les différents objets.
;*
;********************************************************


    section text,code

    moveq   #20,d0
    rts

    XREF    _GermTagList
    dc.l    _GermTagList

    ; L'assembleur de Dice est trop sommaire pour trouver az_JumpOffsets
    ; INCLUDE "AZur:DevelKit/includes/azur.i"
az_JumpOffsets  equ 64


;--------------------------------------------
;   Cette fonction est nécessaire à chaque
;   objet de commande pour qu'il retrouve
;   les fonctions AZ_...()
;--------------------------------------------

    XDEF    __linkazur

__linkazur
    move.l  az_JumpOffsets(a4),d0
    rts

;********************************************************
;*
;*   Ne rien ajouter avant.
;*   A partir d'ici on met ce que l'on veut.
;*
;********************************************************
