#ifndef LC_SELECTIONOPTIONS_H
#define LC_SELECTIONOPTIONS_H

struct LC_SelectionOptions {

    enum ApplyTo {
        Selection,
        Document
    };

    ApplyTo m_applyArea {Document};
    bool m_includeIntoSelectionSet {false};
    bool m_appendToSelectionSet {false};

    LC_SelectionOptions();
};

#endif // LC_SELECTIONOPTIONS_H
