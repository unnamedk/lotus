#include "system_process.hpp"

native::system_process::system_process( std::uint32_t process_id )
{
    this->m_status = PsLookupProcessByProcessId( ( HANDLE )( process_id ), &this->m_obj );
}

native::system_process::~system_process()
{
    if ( this->is_valid() ) {
        ObDereferenceObject( this->m_obj );
    }
}

PEPROCESS native::system_process::get()
{
    return this->m_obj;
}

bool native::system_process::is_valid()
{
    return NT_SUCCESS( this->m_status );
}

long native::system_process::status()
{
    return this->m_status;
}
