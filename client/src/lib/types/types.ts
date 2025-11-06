// export interface Drives: string[]

export interface folder{
    name: string,
    src: string,
    canSee: string | string[],
    canUpload: string | string[],
    canDelete: string | string[],
    canDownload: string | string[],
}

export interface file{
    name: string,
    src: string,
    canSee: string | string[],
    canDownload: string | string[],
}

export interface virtual{
    name: string,
    folders: folder[],
    files: file[],
    canSee: string | string[],
    canDownload: string | string[],
}

